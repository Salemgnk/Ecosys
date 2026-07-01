#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
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

    // Zones durcies : le rendement est faible face au coût de vie, donc les
    // zones ont une capacité de charge limitée -> la surpopulation provoque
    // des famines et régule d'elle-même la population.
    world.addZone(Zone("grasslands", 10.0));
    world.addZone(Zone("desert", 4.0));

    // Species(name, metabolicCost, reproductionThreshold, reproductionCost, aggressiveness)
    auto grazer = std::make_shared<const Species>("Grazer", 2.0, 12.0, 6.0, 0.5);
    auto explorer = std::make_shared<const Species>("Explorer", 3.0, 14.0, 7.0, 0.8);

    // Population initiale plus dense -> pression sur les ressources dès le départ.
    for (int i = 0; i < 4; ++i) {
        world.addOrganism(std::make_unique<Grazer>(grazer, 6.0, "grasslands"));
    }
    for (int i = 0; i < 2; ++i) {
        world.addOrganism(std::make_unique<Grazer>(grazer, 6.0, "desert"));
    }
    for (int i = 0; i < 2; ++i) {
        world.addOrganism(std::make_unique<Explorer>(explorer, 6.0, "desert"));
    }

    Interpreter interpreter;

    // Boucle infinie : tourne jusqu'à Ctrl+C, ou jusqu'à l'extinction totale.
    while (true) {
        std::vector<Event> events = world.tick();

        std::cout << "=== Tick " << world.currentTick()
                  << " (population: " << world.population() << ") ===\n";
        for (const Event& event : events) {
            std::cout << "  " << interpreter.narrate(event) << "\n";
        }

        if (world.population() == 0) {
            std::cout << "The ecosystem has collapsed. Simulation over.\n";
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }

    return 0;
}
