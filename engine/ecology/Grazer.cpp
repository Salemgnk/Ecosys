#include "Grazer.hpp"
#include "World.hpp"

std::unique_ptr<Organism> Grazer::reproduce(std::mt19937& rng)
{
    double energyAfterReproduction = energy_ - genome_.reproductionCost;
    if (energyAfterReproduction < 0.0) {
        return nullptr;
    }
    energy_ = energyAfterReproduction;
    // L'enfant naît avec l'énergie investie (conservation), hérite du génome
    // du parent MUTÉ, et de la génération suivante.
    auto child = std::make_unique<Grazer>(species_, genome_.reproductionCost, zoneName_);
    child->setGenome(genome_.mutated(rng));
    child->setGeneration(generation() + 1);
    return child;
}

void Grazer::act(World& /*world*/, std::vector<Event>& /*out*/)
{
    // Le Grazer broute sur place : il ne se déplace pas.
    // L'alimentation (répartition de l'énergie de la zone entre occupants)
    // est gérée centralement par World::tick(), pour que le rendement d'une
    // zone ne soit distribué qu'une seule fois par tick (énergie conservée).
}