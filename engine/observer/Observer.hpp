#ifndef OBSERVER_HPP
    #define OBSERVER_HPP
    #include <map>
    #include <vector>
    #include "Event.hpp"
    #include "ProcessInfo.hpp"

// Mode miroir : l'écosystème reflète la réalité, il ne l'invente pas.
// L'Observer compare deux snapshots successifs de /proc et traduit les
// différences en Events du même vocabulaire que la simulation :
//   nouveau PID           -> Born
//   PID disparu           -> Died
//   état changé           -> Migrated (running -> sleeping...)
//   mémoire en nette hausse -> Thrived
class Observer {
    private:
        std::map<int, ProcessInfo> previous_;   // clé = pid
        bool hasBaseline_ = false;

        // Hausse relative de mémoire au-delà de laquelle on narre un Thrived.
        static constexpr double kGrowthThreshold = 0.05;   // +5 %

    public:
        // Compare le snapshot au précédent, renvoie les évènements, et
        // mémorise le snapshot pour le prochain appel. Le tout premier appel
        // établit la référence sans rien narrer (tout serait "Born").
        std::vector<Event> observe(const std::vector<ProcessInfo>& snapshot);

        std::size_t population() const;
};

#endif
