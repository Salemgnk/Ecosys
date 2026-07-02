#ifndef PROCESSINFO_HPP
    #define PROCESSINFO_HPP
    #include <string>

// Instantané d'un processus Linux à un instant donné (objet-valeur, comme
// Event) : le strict nécessaire pour identifier, nommer, placer et jauger
// un organisme observé.
struct ProcessInfo {
    int pid = 0;              // identité stable d'un tick à l'autre (le diff)
    int ppid = 0;             // parent : l'arête de lignée du graphe réel
    std::string name;         // le "qui" de la narration (comm)
    std::string zoneName;     // le "où" : l'état du processus (running, sleeping...)
    double energy = 0.0;      // mémoire résidente (RSS) en MiB
    // Champs ajoutés APRÈS energy pour ne pas casser l'init agrégée existante.
    long cpuJiffies = 0;      // utime + stime (ticks CPU cumulés) -> %CPU par delta
    int threads = 0;          // nombre de threads
    double cpu = 0.0;         // %CPU calculé par l'appelant (delta entre snapshots)
    std::string command;      // ligne de commande complète (/proc/pid/cmdline)
};

#endif
