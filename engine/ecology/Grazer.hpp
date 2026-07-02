#ifndef GRAZER_HPP
    #define GRAZER_HPP
    #include "Organism.hpp"

class Grazer : public Organism {
    
    public:
        Grazer(std::shared_ptr<const Species> species, double energy, std::string zoneName):
            Organism(species, energy, zoneName)
        {
        }

        void act(World& world, std::vector<Event>& out) override;
        std::unique_ptr<Organism> reproduce(std::mt19937& rng) override;
};

#endif