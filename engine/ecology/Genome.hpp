#ifndef GENOME_HPP
    #define GENOME_HPP
    #include <algorithm>
    #include <random>

// Le génome d'un organisme : ses traits, propres à LUI (plus au Species
// partagé). C'est ce qui permet à la reproduction de les faire muter, donc
// à l'évolution d'émerger : au fil des générations, les lignées dérivent et
// la sélection naturelle favorise les traits adaptés à l'environnement.
struct Genome {
    double metabolicCost = 1.0;          // énergie perdue par tick
    double reproductionThreshold = 10.0; // énergie requise pour se reproduire
    double reproductionCost = 5.0;       // énergie investie dans l'enfant
    double aggressiveness = 0.5;         // poids dans le partage d'une zone
    double biteSize = 4.0;               // prédateur : énergie prélevée / morsure

    // Copie mutée : chaque trait varie de ~±6 %, borné à des valeurs viables.
    Genome mutated(std::mt19937& rng) const
    {
        std::normal_distribution<double> d(0.0, 0.06);
        auto jitter = [&](double v, double lo, double hi) {
            return std::clamp(v * (1.0 + d(rng)), lo, hi);
        };
        Genome g;
        g.metabolicCost          = jitter(metabolicCost,          0.3, 8.0);
        g.reproductionThreshold  = jitter(reproductionThreshold,  4.0, 40.0);
        g.reproductionCost       = jitter(reproductionCost,       2.0, 20.0);
        g.aggressiveness         = jitter(aggressiveness,         0.05, 1.0);
        g.biteSize               = jitter(biteSize,               1.0, 12.0);
        return g;
    }
};

#endif
