#include "Explorer.hpp"
#include "World.hpp"

#include <random>

void Explorer::act(World& world, std::vector<Event>& out)
{
    // L'Explorer cherche une zone plus riche que la sienne. La migration est
    // EXPLORATOIRE (probabiliste) : sans cela, tous les explorers se ruent sur
    // l'unique zone la plus riche et s'y entassent. L'alimentation est
    // centralisée dans World::tick() après les migrations.
    const Zone& current = world.zoneNamed(zoneName_);
    const Zone* richer = world.richestZoneOtherThan(zoneName_);

    if (richer != nullptr && richer->get_energyYield() > current.get_energyYield()) {
        std::uniform_real_distribution<double> chance(0.0, 1.0);
        if (chance(world.rng()) < 0.3) {
            out.push_back(Event{EventType::Migrated, species_->get_name(), richer->get_name(), ""});
            setZone(richer->get_name());
        }
    }
}

std::unique_ptr<Organism> Explorer::reproduce(std::mt19937& rng)
{
    double energyAfterReproduction = energy_ - genome_.reproductionCost;
    if (energyAfterReproduction < 0.0) {
        return nullptr;
    }
    energy_ = energyAfterReproduction;
    // Même logique que le Grazer : conservation d'énergie, génome muté,
    // génération incrémentée.
    auto child = std::make_unique<Explorer>(species_, genome_.reproductionCost, zoneName_);
    child->setGenome(genome_.mutated(rng));
    child->setGeneration(generation() + 1);
    return child;
}
