#ifndef ORGANISM_HPP
    #define ORGANISM_HPP
    #include <memory>
    #include <random>
    #include <string>
    #include <vector>
    #include "Species.hpp"
    #include "Genome.hpp"
    #include "Event.hpp"

    class World;
    class Organism {
        private:
            int id_ = 0;          // identité stable, attribuée par World à l'ajout
            int generation_ = 0;  // 0 = ensemencé ; enfant = parent + 1

        protected:
            bool alive_ = true;
            std::shared_ptr<const Species> species_;   // archétype (nom)
            Genome genome_;                             // traits propres (évoluent)
            double energy_;
            std::string zoneName_;

        public:
            Organism(std::shared_ptr<const Species> species, double energy, std::string zoneName):
                species_(species), genome_(species->baseGenome()),
                energy_(energy), zoneName_(zoneName)
            {
            }
            virtual ~Organism() = default;

            virtual void act(World& world, std::vector<Event>& out) = 0;
            virtual std::unique_ptr<Organism> reproduce(std::mt19937& rng) = 0;
            virtual bool eatsMeat() const { return false; }   // vrai chez le prédateur

            void metabolize();
            void feed(double amount);
            void drain(double amount);   // prédation : on retire de l'énergie
            bool isAlive() const;
            bool canReproduce() const;

            double energy() const;
            const Species& species() const;
            const Genome& genome() const;
            void setGenome(const Genome& g);
            int generation() const;
            void setGeneration(int g);
            const std::string& zoneName() const;
            void setZone(std::string zoneName);
            int id() const;
            void setId(int id);
    };
#endif
