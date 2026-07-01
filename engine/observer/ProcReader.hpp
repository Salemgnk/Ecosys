#ifndef PROCREADER_HPP
    #define PROCREADER_HPP
    #include <cstddef>
    #include <vector>
    #include "ProcessInfo.hpp"

// Lit /proc et produit un snapshot des processus vivants.
// Ne garde que les topN processus les plus gourmands en mémoire, pour que
// la narration reste lisible (un système en a des centaines).
class ProcReader {
    private:
        std::size_t topN_;

    public:
        explicit ProcReader(std::size_t topN = 15);

        std::vector<ProcessInfo> snapshot() const;

        // Parse le contenu d'un /proc/[pid]/stat. Exposé publiquement pour
        // être testable sans lire le vrai /proc. Renvoie false si illisible.
        static bool parseStatLine(const std::string& statContent, ProcessInfo& out);
};

#endif
