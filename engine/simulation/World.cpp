#include "World.hpp"

World::World(unsigned seed): rng_(seed) {

}

void World::addZone(Zone zone)
{
    zones_.emplace(zone.get_name(), zone);
}

void World::addOrganism(std::unique_ptr<Organism> organism)
{
    organisms_.push_back(std::move(organism));
}

const Zone& World::zoneNamed(const std::string& name) const
{
    return zones_.at(name);
}

std::mt19937& World::rng()
{
    return rng_;
}

unsigned World::currentTick() const
{
    return tick_;
}

// std::vector<Event> World::tick() : implémentée lors de la session dédiée
// (métabolisme -> action -> reproduction -> mort).