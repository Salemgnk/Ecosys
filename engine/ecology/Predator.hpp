#ifndef PREDATOR_HPP
    #define PREDATOR_HPP
    #include "Organism.hpp"

// Le sommet de la chaîne alimentaire : il ne broute pas l'énergie de la zone,
// il chasse les herbivores qui s'y trouvent.
class Predator : public Organism {

    public:
        Predator(std::shared_ptr<const Species> species, double energy, std::string zoneName):
            Organism(species, energy, zoneName)
        {
        }

        bool eatsMeat() const override { return true; }
        void act(World& world, std::vector<Event>& out) override;
        std::unique_ptr<Organism> reproduce(std::mt19937& rng) override;
};

#endif
