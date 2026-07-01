#include <iostream>
#include <memory>
#include <vector>

#include "World.hpp"
#include "Zone.hpp"
#include "Species.hpp"
#include "Grazer.hpp"
#include "Explorer.hpp"
#include "Interpreter.hpp"

int main()
{
    World world(12345);

    // Deux zones : une riche, une pauvre.
    world.addZone(Zone("grasslands", 12.0));
    world.addZone(Zone("desert", 3.0));

    // Species(name, metabolicCost, reproductionThreshold, reproductionCost, aggressiveness)
    auto grazer = std::make_shared<const Species>("Grazer", 2.0, 14.0, 7.0, 0.4);
    auto explorer = std::make_shared<const Species>("Explorer", 3.0, 16.0, 8.0, 0.7);

    // Population initiale.
    world.addOrganism(std::make_unique<Grazer>(grazer, 6.0, "grasslands"));
    world.addOrganism(std::make_unique<Grazer>(grazer, 6.0, "grasslands"));
    world.addOrganism(std::make_unique<Grazer>(grazer, 6.0, "desert"));
    world.addOrganism(std::make_unique<Explorer>(explorer, 6.0, "desert"));

    Interpreter interpreter;

    const int totalTicks = 8;
    for (int t = 0; t < totalTicks; ++t) {
        std::vector<Event> events = world.tick();
        std::cout << "=== Tick " << world.currentTick() << " ===\n";
        for (const Event& event : events) {
            std::cout << "  " << interpreter.narrate(event) << "\n";
        }
    }

    return 0;
}
