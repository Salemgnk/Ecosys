#include "ProcReader.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include <unistd.h>   // sysconf(_SC_PAGESIZE)

namespace {

// L'état brut de /proc ([R], [S], [D]...) devient un nom de zone narratif.
std::string zoneFromState(char state)
{
    switch (state) {
        case 'R': return "running";
        case 'S': return "sleeping";
        case 'D': return "deep-sleep";   // uninterruptible sleep (I/O)
        case 'Z': return "zombie";
        case 'T':
        case 't': return "stopped";
        case 'I': return "idle";
        default:  return "unknown";
    }
}

bool isAllDigits(const std::string& s)
{
    return !s.empty() &&
           std::all_of(s.begin(), s.end(),
                       [](unsigned char c) { return std::isdigit(c); });
}

} // namespace

ProcReader::ProcReader(std::size_t topN): topN_(topN)
{
}

bool ProcReader::parseStatLine(const std::string& statContent, ProcessInfo& out)
{
    // Format de /proc/[pid]/stat : "pid (comm) state ppid ... rss ..."
    // Piège classique : comm peut contenir espaces et parenthèses
    // (ex. "(Web Content)"), donc on repère la DERNIÈRE ')' et on parse
    // le reste à partir de là.
    const std::size_t open = statContent.find('(');
    const std::size_t close = statContent.rfind(')');
    if (open == std::string::npos || close == std::string::npos || close < open) {
        return false;
    }

    try {
        out.pid = std::stoi(statContent.substr(0, open));
    } catch (...) {
        return false;
    }
    out.name = statContent.substr(open + 1, close - open - 1);

    // Après la ')' : state est le 1er champ, ppid le 2e, rss le 22e
    // (champs n°3, 4 et 24 du format stat).
    std::istringstream rest(statContent.substr(close + 1));
    std::string state;
    rest >> state >> out.ppid;
    if (state.empty() || !rest) {
        return false;
    }
    out.zoneName = zoneFromState(state[0]);

    // Champs stat par numéro (après state=3 et ppid=4 déjà lus) :
    //   14 utime · 15 stime · 20 num_threads · 24 rss
    std::string skip;
    for (int i = 0; i < 9; ++i) rest >> skip;     // 5..13
    long utime = 0, stime = 0;
    rest >> utime >> stime;                        // 14, 15
    for (int i = 0; i < 4; ++i) rest >> skip;      // 16..19
    long threads = 0;
    rest >> threads;                               // 20
    for (int i = 0; i < 3; ++i) rest >> skip;      // 21..23
    long rssPages = 0;
    rest >> rssPages;                              // 24
    if (!rest) {
        return false;
    }
    out.cpuJiffies = utime + stime;
    out.threads = static_cast<int>(threads);

    static const long pageSize = sysconf(_SC_PAGESIZE);
    out.energy = static_cast<double>(rssPages) * static_cast<double>(pageSize)
                 / (1024.0 * 1024.0);   // MiB
    return true;
}

std::vector<ProcessInfo> ProcReader::snapshot() const
{
    std::vector<ProcessInfo> processes;

    for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
        const std::string dirName = entry.path().filename().string();
        if (!isAllDigits(dirName)) {
            continue;
        }

        std::ifstream statFile(entry.path() / "stat");
        if (!statFile) {
            continue;   // le processus a pu disparaître entre-temps
        }
        std::string content;
        std::getline(statFile, content);

        ProcessInfo info;
        if (!parseStatLine(content, info)) {
            continue;
        }

        // Ligne de commande complète : /proc/pid/cmdline, arguments séparés
        // par des octets nuls. Vide pour les threads noyau -> on retombe sur
        // le nom entre crochets, comme le fait ps.
        std::ifstream cmdFile(entry.path() / "cmdline", std::ios::binary);
        std::string cmdline((std::istreambuf_iterator<char>(cmdFile)),
                            std::istreambuf_iterator<char>());
        std::replace(cmdline.begin(), cmdline.end(), '\0', ' ');
        while (!cmdline.empty() && cmdline.back() == ' ') {
            cmdline.pop_back();
        }
        info.command = cmdline.empty() ? "[" + info.name + "]" : cmdline;

        processes.push_back(info);
    }

    // Sélection : le top-N par mémoire, PLUS tous leurs ancêtres (même petits),
    // pour que l'arbre des processus reste connexe (le processus navigateur, le
    // shell, systemd... sont légers mais tiennent l'arbre ensemble).
    std::unordered_map<int, std::size_t> byPid;
    for (std::size_t i = 0; i < processes.size(); ++i) {
        byPid[processes[i].pid] = i;
    }

    std::vector<std::size_t> ranked(processes.size());
    for (std::size_t i = 0; i < processes.size(); ++i) ranked[i] = i;
    std::sort(ranked.begin(), ranked.end(),
              [&](std::size_t a, std::size_t b) {
                  return processes[a].energy > processes[b].energy;
              });

    std::unordered_set<int> keep;
    for (std::size_t r = 0; r < ranked.size() && keep.size() < topN_; ++r) {
        keep.insert(processes[ranked[r]].pid);
    }
    // Remonter chaque chaîne de parenté.
    std::vector<int> seeds(keep.begin(), keep.end());
    for (int pid : seeds) {
        int cur = processes[byPid[pid]].ppid;
        while (cur > 1 && byPid.count(cur) && !keep.count(cur)) {
            keep.insert(cur);
            cur = processes[byPid[cur]].ppid;
        }
    }

    std::vector<ProcessInfo> result;
    result.reserve(keep.size());
    for (std::size_t r : ranked) {
        if (keep.count(processes[r].pid)) {
            result.push_back(processes[r]);
        }
    }
    return result;
}
