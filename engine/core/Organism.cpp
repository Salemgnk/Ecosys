#include "Organism.hpp"

void Organism::metabolize()
{
    energy_ -= species_->get_metabolicCost();
}

void Organism::feed(double amount)
{
    energy_ += amount;
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
