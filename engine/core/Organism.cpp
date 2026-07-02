#include "Organism.hpp"

void Organism::metabolize()
{
    energy_ -= species_->get_metabolicCost();
}

void Organism::feed(double amount)
{
    energy_ += amount;
    // Satiété : un organisme ne stocke pas indéfiniment. Plafonner l'énergie
    // désynchronise les reproductions (elles n'arrivent plus toutes en même
    // temps) et amortit le cycle surpopulation/famine.
    const double satiety = species_->get_reproductionThreshold() * 1.5;
    if (energy_ > satiety) {
        energy_ = satiety;
    }
}

bool Organism::canReproduce() const
{
    return energy_ >= species_->get_reproductionThreshold();
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
