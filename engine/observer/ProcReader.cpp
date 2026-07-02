#include "ProcReader.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

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

    long rssPages = 0;
    std::string skip;
    for (int i = 0; i < 19; ++i) {
        rest >> skip;
    }
    rest >> rssPages;
    if (!rest) {
        return false;
    }

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
        if (parseStatLine(content, info)) {
            processes.push_back(info);
        }
    }

    // Top-N par mémoire : la narration suit les organismes les plus massifs.
    std::sort(processes.begin(), processes.end(),
              [](const ProcessInfo& a, const ProcessInfo& b) {
                  return a.energy > b.energy;
              });
    if (processes.size() > topN_) {
        processes.resize(topN_);
    }
    return processes;
}
