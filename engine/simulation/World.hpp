#ifndef WORLD_HPP
    #define WORLD_HPP
    #include <map>
    #include <vector>
    #include <random>
    #include <memory>
    #include <string>
    #include "Organism.hpp"
    #include "Zone.hpp"
    #include "Event.hpp"
    #include "RelationGraph.hpp"

class World {
    private:
        std::vector<std::unique_ptr<Organism>> organisms_;
        std::map<std::string, Zone> zones_;
        std::mt19937 rng_;
        unsigned tick_ = 0;
        int nextId_ = 0;   // attribution des identités stables des organismes
        RelationGraph relations_;

    public:
        explicit World(unsigned seed = 12345);

        // --- Construction du monde ---
        void addZone(Zone zone);
        void addOrganism(std::unique_ptr<Organism> organism);

        // --- Accès ---
        const Zone& zoneNamed(const std::string& name) const;
        const std::map<std::string, Zone>& zones() const;
        const RelationGraph& relations() const;
        std::mt19937& rng();
        unsigned currentTick() const;
        std::size_t population() const;

        // --- Avancement du temps (corps écrit lors de la session dédiée) ---
        std::vector<Event> tick();

        std::vector<Organism*> organismsInZone(const std::string& zoneName) const;
        const Zone* richestZoneOtherThan(const std::string& currentZone) const;

        // --- Pouvoirs divins ---
        void setZoneYield(const std::string& name, double yield);   // bénir / assécher
        void scaleAllYields(double factor);                        // sécheresse / abondance
        std::size_t cullZone(const std::string& name, double fraction);  // foudroyer
        bool hasZone(const std::string& name) const;
};

#endif
