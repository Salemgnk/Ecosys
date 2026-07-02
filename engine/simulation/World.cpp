#include "World.hpp"
#include <algorithm>

World::World(unsigned seed): rng_(seed) {

}

void World::addZone(Zone zone)
{
    zones_.emplace(zone.get_name(), zone);
}

void World::addOrganism(std::unique_ptr<Organism> organism)
{
    organism->setId(++nextId_);
    organisms_.push_back(std::move(organism));
}

const Zone& World::zoneNamed(const std::string& name) const
{
    return zones_.at(name);
}

const std::map<std::string, Zone>& World::zones() const
{
    return zones_;
}

const RelationGraph& World::relations() const
{
    return relations_;
}

std::mt19937& World::rng()
{
    return rng_;
}

unsigned World::currentTick() const
{
    return tick_;
}

std::size_t World::population() const
{
    return organisms_.size();
}

std::vector<Event> World::tick()
{
    std::vector<Event> events;

    // 1. Métabolisme : chaque organisme vivant paie son coût d'entretien.
    for (auto& organism : organisms_) {
        if (organism->isAlive()) {
            organism->metabolize();
        }
    }

    // 2. Comportement propre à l'espèce (polymorphe) : ici, les déplacements.
    //    L'Explorer peut migrer ; le Grazer reste sur place.
    for (auto& organism : organisms_) {
        if (organism->isAlive()) {
            organism->act(*this, events);
        }
    }

    // 3. Alimentation : pour chaque zone, le rendement se répartit entre ses
    //    occupants au prorata de leur agressivité. Fait UNE fois par zone
    //    (après les migrations) -> l'énergie de la zone est conservée.
    //    Au passage, on reconstruit les arêtes de compétition du graphe.
    relations_.clearCompetition();
    for (const auto& [name, zone] : zones_) {
        std::vector<Organism*> occupants = organismsInZone(name);
        if (occupants.empty()) {
            continue;
        }
        for (std::size_t i = 0; i < occupants.size(); ++i) {
            for (std::size_t j = i + 1; j < occupants.size(); ++j) {
                relations_.addCompetition(occupants[i]->id(), occupants[j]->id());
            }
        }

        double totalAggressiveness = 0.0;
        for (Organism* occupant : occupants) {
            totalAggressiveness += occupant->species().get_aggressiveness();
        }

        for (Organism* occupant : occupants) {
            double share = (totalAggressiveness > 0.0)
                ? zone.get_energyYield() * (occupant->species().get_aggressiveness() / totalAggressiveness)
                : zone.get_energyYield() / static_cast<double>(occupants.size());
            occupant->feed(share);

            EventType type = (occupants.size() > 1) ? EventType::Competed : EventType::Thrived;
            events.push_back(Event{type, occupant->species().get_name(), name, ""});
        }
    }

    // 4. Reproduction : les enfants sont collectés à part pour ne pas modifier
    //    organisms_ pendant qu'on le parcourt (invalidation d'itérateurs).
    //    On retient le parent de chacun pour tracer la lignée dans le graphe.
    std::vector<std::unique_ptr<Organism>> newborns;
    std::vector<int> newbornParents;
    for (auto& organism : organisms_) {
        if (organism->isAlive() && organism->canReproduce()) {
            std::unique_ptr<Organism> child = organism->reproduce();
            if (child != nullptr) {
                events.push_back(Event{EventType::Reproduced, organism->species().get_name(), organism->zoneName(), ""});
                events.push_back(Event{EventType::Born, child->species().get_name(), child->zoneName(), ""});
                newborns.push_back(std::move(child));
                newbornParents.push_back(organism->id());
            }
        }
    }
    for (std::size_t i = 0; i < newborns.size(); ++i) {
        Organism* child = newborns[i].get();
        addOrganism(std::move(newborns[i]));   // attribue l'id de l'enfant
        relations_.addLineage(newbornParents[i], child->id());
    }

    // 5. Mort : on signale les morts par famine, on les retire du graphe
    //    de relations, puis du monde.
    for (auto& organism : organisms_) {
        if (!organism->isAlive()) {
            events.push_back(Event{EventType::Starved, organism->species().get_name(), organism->zoneName(), ""});
            relations_.removeOrganism(organism->id());
        }
    }
    organisms_.erase(
        std::remove_if(organisms_.begin(), organisms_.end(),
                       [](const std::unique_ptr<Organism>& o) { return !o->isAlive(); }),
        organisms_.end());

    ++tick_;
    return events;
}

const Zone* World::richestZoneOtherThan(const std::string& currentZone) const
{
    const Zone* best = nullptr;
    for (const auto& [name, zone] : zones_) {
        if (name == currentZone) {
            continue;
        }
        if (best == nullptr || zone.get_energyYield() > best->get_energyYield()) {
            best = &zone;
        }
    }
    return best;
}

std::vector<Organism*> World::organismsInZone(const std::string& zoneName) const
{
    std::vector<Organism*> result;
    for (const auto& organism : organisms_) {
        if (organism->isAlive() && organism->zoneName() == zoneName) {
            result.push_back(organism.get());
        }
    }
    return result;
}