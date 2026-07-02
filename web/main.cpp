#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstring>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <unistd.h>   // sysconf(_SC_CLK_TCK)

#include <httplib.h>

#include "World.hpp"
#include "Zone.hpp"
#include "Species.hpp"
#include "Grazer.hpp"
#include "Explorer.hpp"
#include "Predator.hpp"
#include "Interpreter.hpp"
#include "Observer.hpp"
#include "ProcReader.hpp"
#include "RelationGraph.hpp"

// ecosys-web : la "vue Dieu" servie sur localhost.
// Le monde (simulé, ou miroir de /proc avec --observe) tourne dans un thread ;
// le frontend Canvas récupère l'état complet en JSON via GET /state.

namespace {

std::mutex g_stateMutex;
std::string g_stateJson = "{}";
bool g_allowKill = false;      // /kill n'existe que si --allow-kill
std::size_t g_topN = 50;       // processus suivis (--top N / --all)

void publishState(std::string json)
{
    std::lock_guard<std::mutex> lock(g_stateMutex);
    g_stateJson = std::move(json);
}

std::string jsonEscape(const std::string& s)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\t': out += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) >= 0x20) {
                    out += c;
                }
        }
    }
    return out;
}

const char* eventTypeName(EventType type)
{
    switch (type) {
        case EventType::Born:       return "born";
        case EventType::Died:       return "died";
        case EventType::Reproduced: return "reproduced";
        case EventType::Migrated:   return "migrated";
        case EventType::Competed:   return "competed";
        case EventType::Thrived:    return "thrived";
        case EventType::Starved:    return "starved";
    }
    return "unknown";
}

// Journal roulant des derniers évènements, numérotés pour que le frontend
// n'affiche chaque histoire qu'une seule fois malgré le polling.
struct EventLog {
    std::deque<std::pair<long, std::string>> entries;   // (seq, json)
    long nextSeq = 1;

    void push(const Event& event, const Interpreter& interpreter)
    {
        std::ostringstream json;
        json << "{\"seq\":" << nextSeq
             << ",\"type\":\"" << eventTypeName(event.type)
             << "\",\"text\":\"" << jsonEscape(interpreter.narrate(event)) << "\"}";
        entries.emplace_back(nextSeq, json.str());
        ++nextSeq;
        while (entries.size() > 40) {
            entries.pop_front();
        }
    }

    void serialize(std::ostringstream& out) const
    {
        out << "[";
        for (std::size_t i = 0; i < entries.size(); ++i) {
            if (i > 0) out << ",";
            out << entries[i].second;
        }
        out << "]";
    }
};

struct OrganismView {
    int id;
    std::string name;
    std::string zone;
    double energy;
    double maxEnergy;   // référence pour la jauge (seuil de repro / max observé)
    double cpu = 0.0;   // %CPU (observation) ; 0 en simulation
    int threads = 0;
    int ppid = 0;
    std::string command;
};

struct ZoneView {
    std::string name;
    double yield;
};

std::string buildStateJson(const char* mode, unsigned tick,
                           const std::vector<ZoneView>& zones,
                           const std::vector<OrganismView>& organisms,
                           const std::vector<Relation>& relations,
                           const EventLog& log)
{
    std::ostringstream out;
    out << "{\"mode\":\"" << mode << "\",\"tick\":" << tick
        << ",\"canKill\":" << (g_allowKill ? "true" : "false")
        << ",\"population\":" << organisms.size() << ",\"zones\":[";
    for (std::size_t i = 0; i < zones.size(); ++i) {
        if (i > 0) out << ",";
        out << "{\"name\":\"" << jsonEscape(zones[i].name)
            << "\",\"yield\":" << zones[i].yield << "}";
    }
    out << "],\"organisms\":[";
    for (std::size_t i = 0; i < organisms.size(); ++i) {
        const OrganismView& o = organisms[i];
        if (i > 0) out << ",";
        out << "{\"id\":" << o.id
            << ",\"name\":\"" << jsonEscape(o.name)
            << "\",\"zone\":\"" << jsonEscape(o.zone)
            << "\",\"energy\":" << o.energy
            << ",\"max\":" << o.maxEnergy
            << ",\"cpu\":" << o.cpu
            << ",\"threads\":" << o.threads
            << ",\"ppid\":" << o.ppid
            << ",\"command\":\"" << jsonEscape(o.command) << "\"}";
    }
    out << "],\"relations\":[";
    for (std::size_t i = 0; i < relations.size(); ++i) {
        const Relation& r = relations[i];
        if (i > 0) out << ",";
        out << "{\"from\":" << r.from << ",\"to\":" << r.to
            << ",\"type\":\""
            << (r.type == RelationType::Lineage ? "lineage" : "competition")
            << "\"}";
    }
    out << "],\"events\":";
    log.serialize(out);
    out << "}";
    return out.str();
}

// --- Mode simulation : le monde durci de la démo, un tick toutes les 600 ms.
void simulationLoop()
{
    // Monde de jeu : cinq biomes aux rendements variés, une chaîne alimentaire
    // (herbivores broutent, prédateurs chassent) et des génomes qui mutent.
    World world(12345);
    world.addZone(Zone("forest", 22.0));
    world.addZone(Zone("plains", 16.0));
    world.addZone(Zone("shoreline", 12.0));
    world.addZone(Zone("savanna", 8.0));
    world.addZone(Zone("desert", 5.0));

    // Proies qui se reproduisent vite (seuils bas) -> elles encaissent la
    // prédation ; prédateurs peu nombreux et modérés -> le cycle oscille au
    // lieu de s'effondrer d'un coup.
    auto grazer   = std::make_shared<const Species>("Grazer", 1.8, 9.0, 4.5, 0.5);
    auto explorer = std::make_shared<const Species>("Explorer", 2.4, 11.0, 5.5, 0.8);
    auto predator = std::make_shared<const Species>("Predator", 3.0, 24.0, 12.0, 0.9, 5.0);

    const char* biomes[] = {"forest", "plains", "shoreline", "savanna", "desert"};
    for (const char* b : biomes) {
        for (int i = 0; i < 5; ++i) world.addOrganism(std::make_unique<Grazer>(grazer, 6.0, b));
        for (int i = 0; i < 2; ++i) world.addOrganism(std::make_unique<Explorer>(explorer, 6.0, b));
    }
    for (const char* b : {"forest", "plains"}) {
        world.addOrganism(std::make_unique<Predator>(predator, 12.0, b));
    }

    Interpreter interpreter;
    EventLog log;

    while (true) {
        for (const Event& event : world.tick()) {
            log.push(event, interpreter);
        }

        std::vector<ZoneView> zones;
        std::vector<OrganismView> organisms;
        for (const auto& [name, zone] : world.zones()) {
            zones.push_back({name, zone.get_energyYield()});
            for (Organism* o : world.organismsInZone(name)) {
                OrganismView view;
                view.id = o->id();
                view.name = o->species().get_name();
                view.zone = name;
                view.energy = o->energy();
                view.maxEnergy = o->genome().reproductionThreshold;
                view.command = o->species().get_name();
                organisms.push_back(std::move(view));
            }
        }
        publishState(buildStateJson("simulation", world.currentTick(),
                                    zones, organisms, world.relations().all(),
                                    log));

        std::this_thread::sleep_for(std::chrono::milliseconds(600));
    }
}

// --- Mode observation : miroir de /proc, un snapshot par seconde.
void observationLoop()
{
    ProcReader reader(g_topN);
    Observer observer;
    Interpreter interpreter;
    EventLog log;
    unsigned tick = 0;

    const double clockHz = static_cast<double>(sysconf(_SC_CLK_TCK));
    std::map<int, long> prevJiffies;
    auto prevTime = std::chrono::steady_clock::now();

    while (true) {
        std::vector<ProcessInfo> snapshot = reader.snapshot();
        for (const Event& event : observer.observe(snapshot)) {
            log.push(event, interpreter);
        }
        ++tick;

        // %CPU = variation des jiffies CPU du processus, rapportée au temps
        // écoulé (et à la fréquence d'horloge du noyau).
        const auto now = std::chrono::steady_clock::now();
        const double dt = std::chrono::duration<double>(now - prevTime).count();
        prevTime = now;
        std::map<int, long> currentJiffies;

        // Les zones sont les états réellement présents ; leur "yield" affiché
        // est la mémoire totale (MiB) des processus qui l'occupent.
        std::vector<ZoneView> zones;
        std::vector<OrganismView> organisms;
        double maxEnergy = 1.0;
        for (const ProcessInfo& proc : snapshot) {
            maxEnergy = std::max(maxEnergy, proc.energy);
        }
        for (const ProcessInfo& proc : snapshot) {
            bool zoneKnown = false;
            for (ZoneView& zone : zones) {
                if (zone.name == proc.zoneName) {
                    zone.yield += proc.energy;
                    zoneKnown = true;
                    break;
                }
            }
            if (!zoneKnown) {
                zones.push_back({proc.zoneName, proc.energy});
            }

            currentJiffies[proc.pid] = proc.cpuJiffies;
            double cpu = 0.0;
            auto it = prevJiffies.find(proc.pid);
            if (it != prevJiffies.end() && dt > 0.0 && clockHz > 0.0) {
                cpu = (proc.cpuJiffies - it->second) / clockHz / dt * 100.0;
                if (cpu < 0.0) cpu = 0.0;
            }

            OrganismView view;
            view.id = proc.pid;
            view.name = proc.name;
            view.zone = proc.zoneName;
            view.energy = proc.energy;
            view.maxEnergy = maxEnergy;
            view.cpu = cpu;
            view.threads = proc.threads;
            view.ppid = proc.ppid;
            view.command = proc.command;
            organisms.push_back(std::move(view));
        }
        prevJiffies = std::move(currentJiffies);

        // Le graphe réel : l'arbre des processus (arête quand parent et
        // enfant sont tous deux visibles dans le top-N).
        std::vector<Relation> relations;
        for (const ProcessInfo& proc : snapshot) {
            for (const ProcessInfo& candidate : snapshot) {
                if (candidate.pid == proc.ppid) {
                    relations.push_back(
                        Relation{proc.ppid, proc.pid, RelationType::Lineage});
                    break;
                }
            }
        }
        publishState(buildStateJson("observation", tick, zones, organisms,
                                    relations, log));

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

} // namespace

int main(int argc, char** argv)
{
    bool observe = false;
    int port = 8080;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--observe") == 0) {
            observe = true;
        } else if (std::strcmp(argv[i], "--allow-kill") == 0) {
            g_allowKill = true;
        } else if (std::strcmp(argv[i], "--all") == 0) {
            g_topN = static_cast<std::size_t>(-1);   // tous les processus
        } else if (std::strcmp(argv[i], "--top") == 0 && i + 1 < argc) {
            g_topN = static_cast<std::size_t>(std::stoul(argv[++i]));
        } else if (std::strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        }
    }

    std::thread worldThread(observe ? observationLoop : simulationLoop);
    worldThread.detach();

    httplib::Server server;
    server.set_mount_point("/", ECOSYS_WEB_STATIC);
    server.Get("/state", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_stateMutex);
        res.set_content(g_stateJson, "application/json");
    });

    // Terminer un processus : désactivé sauf --allow-kill. Envoie SIGTERM
    // (le noyau refusera si l'utilisateur n'en a pas le droit).
    if (g_allowKill && observe) {
        server.Post("/kill", [](const httplib::Request& req, httplib::Response& res) {
            int pid = 0;
            if (req.has_param("pid")) {
                try { pid = std::stoi(req.get_param_value("pid")); } catch (...) { pid = 0; }
            }
            bool ok = pid > 1 && ::kill(static_cast<pid_t>(pid), SIGTERM) == 0;
            res.set_content(std::string("{\"ok\":") + (ok ? "true" : "false") + "}",
                            "application/json");
        });
    }

    std::cout << "Ecosys monitor — http://localhost:" << port
              << (observe ? "  (observing /proc)" : "  (simulation)")
              << (g_allowKill ? "  [kill enabled]" : "") << "\n";
    if (!server.listen("127.0.0.1", port)) {
        std::cerr << "Could not bind port " << port << "\n";
        return 1;
    }
    return 0;
}
