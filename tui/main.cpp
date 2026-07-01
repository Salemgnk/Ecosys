#include <algorithm>
#include <chrono>
#include <cstdio>
#include <deque>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#include "World.hpp"
#include "Zone.hpp"
#include "Species.hpp"
#include "Grazer.hpp"
#include "Explorer.hpp"
#include "Interpreter.hpp"

using namespace ftxui;

namespace {

std::string formatEnergy(double value)
{
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%.1f", value);
    return buffer;
}

// Chaque type d'évènement a son icône et sa couleur dans le flux.
Color eventColor(EventType type)
{
    switch (type) {
        case EventType::Born:
        case EventType::Reproduced: return Color::Green;
        case EventType::Thrived:    return Color::GreenLight;
        case EventType::Migrated:   return Color::Cyan;
        case EventType::Competed:   return Color::Yellow;
        case EventType::Starved:
        case EventType::Died:       return Color::Red;
    }
    return Color::White;
}

std::string eventIcon(EventType type)
{
    switch (type) {
        case EventType::Born:       return "🌱";
        case EventType::Reproduced: return "🐣";
        case EventType::Thrived:    return "✨";
        case EventType::Migrated:   return "🧭";
        case EventType::Competed:   return "⚔️ ";
        case EventType::Starved:
        case EventType::Died:       return "💀";
    }
    return "  ";
}

// Un panneau par zone : ses organismes avec une barre d'énergie.
// Barre pleine = énergie au seuil de reproduction.
Element zonePanel(const World& world, const std::string& name, const Zone& zone)
{
    Elements rows;
    for (Organism* organism : world.organismsInZone(name)) {
        const double threshold = organism->species().get_reproductionThreshold();
        const double ratio = std::clamp(
            threshold > 0.0 ? organism->energy() / threshold : 0.0, 0.0, 1.0);
        const Color barColor = ratio > 0.66 ? Color::Green
                             : ratio > 0.33 ? Color::Yellow
                                            : Color::Red;
        rows.push_back(hbox({
            text(organism->species().get_name()) | size(WIDTH, EQUAL, 10),
            gauge(ratio) | color(barColor) | flex,
            text(" " + formatEnergy(organism->energy())) | dim,
        }));
    }
    if (rows.empty()) {
        rows.push_back(text("(uninhabited)") | dim | center);
    }
    return window(
        text(" " + name + " · yield " + formatEnergy(zone.get_energyYield()) + " ") | bold,
        vbox(std::move(rows))) | flex;
}

} // namespace

int main()
{
    // Même monde que la démo CLI : durci, auto-régulé.
    World world(12345);
    world.addZone(Zone("grasslands", 10.0));
    world.addZone(Zone("desert", 4.0));

    auto grazer = std::make_shared<const Species>("Grazer", 2.0, 12.0, 6.0, 0.5);
    auto explorer = std::make_shared<const Species>("Explorer", 3.0, 14.0, 7.0, 0.8);
    for (int i = 0; i < 4; ++i) {
        world.addOrganism(std::make_unique<Grazer>(grazer, 6.0, "grasslands"));
    }
    for (int i = 0; i < 2; ++i) {
        world.addOrganism(std::make_unique<Grazer>(grazer, 6.0, "desert"));
        world.addOrganism(std::make_unique<Explorer>(explorer, 6.0, "desert"));
    }

    Interpreter interpreter;
    // Flux des derniers évènements ; les doublons consécutifs (ex. la même
    // compétition répétée par tous les occupants d'une zone) sont regroupés
    // en une ligne avec un compteur ×N, sinon le flux ne raconte plus rien.
    std::deque<std::pair<Event, int>> feed;
    constexpr std::size_t kFeedSize = 8;
    std::string resetPosition;              // boucle d'animation FTXUI

    auto sameStory = [](const Event& a, const Event& b) {
        return a.type == b.type && a.speciesName == b.speciesName &&
               a.zoneName == b.zoneName;
    };

    while (true) {
        std::vector<Event> events = world.tick();
        for (Event& event : events) {
            if (!feed.empty() && sameStory(feed.back().first, event)) {
                ++feed.back().second;
            } else {
                feed.emplace_back(std::move(event), 1);
            }
        }
        while (feed.size() > kFeedSize) {
            feed.pop_front();
        }

        // Bandeau d'état.
        auto header = hbox({
            text(" ECOSYS 🌱 ") | bold | color(Color::Green),
            filler(),
            text("tick " + std::to_string(world.currentTick())) | dim,
            text("  •  population " + std::to_string(world.population()) + " "),
        });

        // Un panneau par zone, côte à côte.
        Elements zonePanels;
        for (const auto& [name, zone] : world.zones()) {
            zonePanels.push_back(zonePanel(world, name, zone));
        }

        // Flux narratif.
        Elements feedRows;
        for (const auto& [event, count] : feed) {
            Elements line = {
                text(eventIcon(event.type) + " "),
                text(interpreter.narrate(event)) | color(eventColor(event.type)),
            };
            if (count > 1) {
                line.push_back(text("  ×" + std::to_string(count)) | dim);
            }
            feedRows.push_back(hbox(std::move(line)));
        }
        if (feedRows.empty()) {
            feedRows.push_back(text("(the world is quiet)") | dim);
        }

        auto document = vbox({
            header,
            separator(),
            hbox(std::move(zonePanels)),
            window(text(" events ") | bold, vbox(std::move(feedRows))),
        }) | border;

        auto screen = Screen::Create(Dimension::Full(), Dimension::Fit(document));
        Render(screen, document);
        std::cout << resetPosition;
        screen.Print();
        std::cout.flush();
        resetPosition = screen.ResetPosition();

        if (world.population() == 0) {
            std::cout << "\nThe ecosystem has collapsed. Simulation over.\n";
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }

    return 0;
}
