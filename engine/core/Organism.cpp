#include "Organism.hpp"

void Organism::metabolize()
{
    energy_ -= genome_.metabolicCost;
}

void Organism::feed(double amount)
{
    energy_ += amount;
    // Satiété : un organisme ne stocke pas indéfiniment. Plafonner l'énergie
    // désynchronise les reproductions (elles n'arrivent plus toutes en même
    // temps) et amortit le cycle surpopulation/famine.
    const double satiety = genome_.reproductionThreshold * 1.5;
    if (energy_ > satiety) {
        energy_ = satiety;
    }
}

void Organism::drain(double amount)
{
    energy_ -= amount;   // proie mordue ; isAlive() gère la mort à energy <= 0
}

bool Organism::canReproduce() const
{
    return energy_ >= genome_.reproductionThreshold;
}

double Organism::energy() const
{
    return energy_;
}

const std::string& Organism::zoneName() const
{
    return zoneName_;
}

const Species& Organism::species() const
{
    return *species_;
}

const Genome& Organism::genome() const
{
    return genome_;
}

void Organism::setGenome(const Genome& g)
{
    genome_ = g;
}

int Organism::generation() const
{
    return generation_;
}

void Organism::setGeneration(int g)
{
    generation_ = g;
}

void Organism::setZone(std::string zoneName)
{
    zoneName_ = zoneName;
}

bool Organism::isAlive() const
{
    return alive_ && energy_ > 0.0;
}

int Organism::id() const
{
    return id_;
}

void Organism::setId(int id)
{
    id_ = id;
}
