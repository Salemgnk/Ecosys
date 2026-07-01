#include "Grazer.hpp"
#include "World.hpp"

std::unique_ptr<Organism> Grazer::reproduce()
{
    double energyAfterReproduction = energy_ - species_->get_reproductionCost();
    if (energyAfterReproduction < 0.0) {
        return nullptr;
    }
    energy_ = energyAfterReproduction;
    // L'enfant naît avec l'énergie que le parent vient d'investir (reproductionCost)
    // -> l'énergie totale est conservée (parent -cost, enfant +cost).
    return std::make_unique<Grazer>(species_, species_->get_reproductionCost(), zoneName_);
}

void Grazer::act(World& /*world*/, std::vector<Event>& /*out*/)
{
    // Le Grazer broute sur place : il ne se déplace pas.
    // L'alimentation (répartition de l'énergie de la zone entre occupants)
    // est gérée centralement par World::tick(), pour que le rendement d'une
    // zone ne soit distribué qu'une seule fois par tick (énergie conservée).
}