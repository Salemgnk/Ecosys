#include "Predator.hpp"
#include "World.hpp"

#include <algorithm>

void Predator::act(World& world, std::vector<Event>& out)
{
    // Chasse : dans sa zone, il mord la proie herbivore la plus faible.
    Organism* prey = nullptr;
    for (Organism* o : world.organismsInZone(zoneName_)) {
        if (o == this || o->eatsMeat() || !o->isAlive()) {
            continue;
        }
        if (prey == nullptr || o->energy() < prey->energy()) {
            prey = o;
        }
    }
    if (prey == nullptr) {
        return;   // pas de proie : le prédateur jeûne (et paiera son métabolisme)
    }

    const double bite = std::min(prey->energy() + 0.001, genome_.biteSize);
    prey->drain(bite);
    feed(bite);
    out.push_back(Event{EventType::Competed, species_->get_name(), zoneName_,
                        prey->species().get_name()});
}

std::unique_ptr<Organism> Predator::reproduce(std::mt19937& rng)
{
    double energyAfterReproduction = energy_ - genome_.reproductionCost;
    if (energyAfterReproduction < 0.0) {
        return nullptr;
    }
    energy_ = energyAfterReproduction;
    auto child = std::make_unique<Predator>(species_, genome_.reproductionCost, zoneName_);
    child->setGenome(genome_.mutated(rng));
    child->setGeneration(generation() + 1);
    return child;
}
