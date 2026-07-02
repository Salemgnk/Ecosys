#ifndef EXPLORER_HPP
    #define EXPLORER_HPP
    #include "Organism.hpp"

class Explorer : public Organism {

    public:
        Explorer(std::shared_ptr<const Species> species, double energy, std::string zoneName):
            Organism(species, energy, zoneName)
        {
        }

        void act(World& world, std::vector<Event>& out) override;
        std::unique_ptr<Organism> reproduce(std::mt19937& rng) override;
};

#endif
