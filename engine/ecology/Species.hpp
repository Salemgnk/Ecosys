#ifndef SPECIES_HPP
    #define SPECIES_HPP
    #include <string>
    #include "Genome.hpp"

class Species {
    private : 
        std::string name_;
        double metabolicCost_;
        double reproductionThreshold_;
        double reproductionCost_;
        double aggressiveness_;
        
    public :
        Species(std::string name, double metabolicCost, double reproductionThreshold, double reproductionCost, double aggressiveness):
            name_(name), metabolicCost_(metabolicCost), reproductionThreshold_(reproductionThreshold), reproductionCost_(reproductionCost), aggressiveness_(aggressiveness)
        {

        }
        const std::string& get_name(void) const 
        {
            return name_;
        }
        double get_metabolicCost(void) const 
        {
            return metabolicCost_;
        }
        double get_reproductionThreshold(void) const
        {
            return reproductionThreshold_;
        }
        double get_reproductionCost(void) const
        {
            return reproductionCost_;
        }
        double get_aggressiveness(void) const
        {
            return aggressiveness_;
        }
        // Génome de départ de l'archétype (les organismes le copient à la
        // naissance, puis il mute au fil des générations).
        Genome baseGenome(void) const
        {
            Genome g;
            g.metabolicCost = metabolicCost_;
            g.reproductionThreshold = reproductionThreshold_;
            g.reproductionCost = reproductionCost_;
            g.aggressiveness = aggressiveness_;
            return g;
        }
};

#endif