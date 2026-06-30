#ifndef ORGANISM_HPP
    #define ORGANISM_HPP
    #include <memory>
    #include <string>
    #include <vector>
    #include "Species.hpp"
    #include "Event.hpp"

    class World;
    class Organism {
        protected:
            bool alive_ = true;
            std::shared_ptr<const Species> species_;
            double energy_;
            std::string zoneName_;
        
        public:
            Organism(std::shared_ptr<const Species> species, double energy, std::string zoneName):
                species_(species), energy_(energy), zoneName_(zoneName)
            {
            }
            virtual ~Organism() = default;
            
            virtual void act(World& world, std::vector<Event>& out) = 0;
            virtual std::unique_ptr<Organism> reproduce() = 0;

            void metabolize();
            void feed(double amount);
            bool isAlive() const;
            bool canReproduce() const;

            double energy() const;
            const Species& species() const;
            const std::string& zoneName() const;
            void setZone(std::string zoneName);
    };
#endif
