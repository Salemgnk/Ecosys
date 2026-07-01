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

class World {
    private:
        std::vector<std::unique_ptr<Organism>> organisms_;
        std::map<std::string, Zone> zones_;
        std::mt19937 rng_;
        unsigned tick_ = 0;

    public:
        explicit World(unsigned seed = 12345);

        // --- Construction du monde ---
        void addZone(Zone zone);
        void addOrganism(std::unique_ptr<Organism> organism);

        // --- Accès ---
        const Zone& zoneNamed(const std::string& name) const;
        const std::map<std::string, Zone>& zones() const;
        std::mt19937& rng();
        unsigned currentTick() const;
        std::size_t population() const;

        // --- Avancement du temps (corps écrit lors de la session dédiée) ---
        std::vector<Event> tick();

        std::vector<Organism*> organismsInZone(const std::string& zoneName) const;
        const Zone* richestZoneOtherThan(const std::string& currentZone) const;
};

#endif
