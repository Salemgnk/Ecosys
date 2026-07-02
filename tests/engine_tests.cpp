#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "Species.hpp"
#include "Zone.hpp"
#include "Grazer.hpp"
#include "Explorer.hpp"
#include "World.hpp"
#include "Interpreter.hpp"

// Petit harnais sans dépendance : REQUIRE ne s'appuie pas sur assert()
// (qui serait désactivé en build Release via NDEBUG).
static int g_failures = 0;
#define REQUIRE(cond)                                                       \
    do {                                                                    \
        if (!(cond)) {                                                      \
            std::cerr << "FAILED: " #cond " (line " << __LINE__ << ")\n";   \
            ++g_failures;                                                   \
        }                                                                   \
    } while (0)

static std::shared_ptr<const Species> makeSpecies(double metabolic, double threshold,
                                                  double cost, double aggr)
{
    return std::make_shared<const Species>("Test", metabolic, threshold, cost, aggr);
}

static void test_metabolize_costs_energy()
{
    Grazer g(makeSpecies(2.0, 100.0, 1.0, 0.5), 10.0, "z");
    g.metabolize();
    REQUIRE(g.energy() == 8.0);
}

static void test_organism_dies_at_zero_energy()
{
    Grazer g(makeSpecies(2.0, 100.0, 1.0, 0.5), 2.0, "z");
    REQUIRE(g.isAlive());
    g.metabolize(); // 2 - 2 = 0
    REQUIRE(!g.isAlive());
}

static void test_can_reproduce_threshold()
{
    auto sp = makeSpecies(1.0, 5.0, 2.0, 0.5);
    REQUIRE(!Grazer(sp, 4.0, "z").canReproduce()); // sous le seuil
    REQUIRE(Grazer(sp, 6.0, "z").canReproduce());  // au-dessus
}

static void test_reproduce_conserves_energy()
{
    Grazer parent(makeSpecies(1.0, 5.0, 3.0, 0.5), 10.0, "z");
    std::unique_ptr<Organism> child = parent.reproduce();
    REQUIRE(child != nullptr);
    REQUIRE(parent.energy() == 7.0);  // parent -cost
    REQUIRE(child->energy() == 3.0);  // enfant +cost
    // total conservé : 7 + 3 == 10 (l'énergie de départ)
}

static void test_reproduce_fails_when_too_poor()
{
    Grazer parent(makeSpecies(1.0, 5.0, 8.0, 0.5), 5.0, "z"); // 5 - 8 < 0
    std::unique_ptr<Organism> child = parent.reproduce();
    REQUIRE(child == nullptr);
    REQUIRE(parent.energy() == 5.0); // inchangé
}

static std::vector<std::string> runStory(unsigned seed)
{
    World world(seed);
    world.addZone(Zone("rich", 12.0));
    world.addZone(Zone("poor", 3.0));
    auto grazer = std::make_shared<const Species>("Grazer", 2.0, 14.0, 7.0, 0.4);
    auto explorer = std::make_shared<const Species>("Explorer", 3.0, 16.0, 8.0, 0.7);
    world.addOrganism(std::make_unique<Grazer>(grazer, 6.0, "rich"));
    world.addOrganism(std::make_unique<Grazer>(grazer, 6.0, "poor"));
    world.addOrganism(std::make_unique<Explorer>(explorer, 6.0, "poor"));

    Interpreter interp;
    std::vector<std::string> lines;
    for (int t = 0; t < 6; ++t) {
        for (const Event& e : world.tick()) {
            lines.push_back(interp.narrate(e));
        }
    }
    return lines;
}

static void test_simulation_is_deterministic()
{
    REQUIRE(runStory(42) == runStory(42));
}

static void test_relation_graph_records_lineage_and_competition()
{
    World world(1);
    world.addZone(Zone("z", 10.0));
    // Deux organismes dans la même zone ; le premier est prêt à se reproduire.
    auto sp = std::make_shared<const Species>("Grazer", 1.0, 5.0, 2.0, 0.5);
    world.addOrganism(std::make_unique<Grazer>(sp, 20.0, "z"));
    world.addOrganism(std::make_unique<Grazer>(sp, 20.0, "z"));

    world.tick();
    const RelationGraph& graph = world.relations();
    REQUIRE(graph.lineage().size() == 2);       // chacun a fait un enfant
    REQUIRE(graph.lineage()[0].from == 1);      // parent -> enfant
    REQUIRE(!graph.competition().empty());      // ils partagent la zone
}

static void test_dead_organism_leaves_the_graph()
{
    RelationGraph graph;
    graph.addLineage(1, 2);
    graph.addCompetition(2, 3);
    graph.removeOrganism(2);
    REQUIRE(graph.lineage().empty());
    REQUIRE(graph.competition().empty());
    REQUIRE(graph.all().empty());
}

static void test_explorer_migrates_to_richer_zone()
{
    World world(1);
    world.addZone(Zone("rich", 20.0));
    world.addZone(Zone("poor", 2.0));
    auto explorer = std::make_shared<const Species>("Explorer", 1.0, 100.0, 1.0, 0.5);
    world.addOrganism(std::make_unique<Explorer>(explorer, 10.0, "poor"));

    world.tick();
    REQUIRE(world.organismsInZone("rich").size() == 1);
    REQUIRE(world.organismsInZone("poor").empty());
}

int main()
{
    test_metabolize_costs_energy();
    test_organism_dies_at_zero_energy();
    test_can_reproduce_threshold();
    test_reproduce_conserves_energy();
    test_reproduce_fails_when_too_poor();
    test_simulation_is_deterministic();
    test_relation_graph_records_lineage_and_competition();
    test_dead_organism_leaves_the_graph();
    test_explorer_migrates_to_richer_zone();

    if (g_failures > 0) {
        std::cerr << g_failures << " test(s) failed.\n";
        return 1;
    }
    std::cout << "All engine tests passed.\n";
    return 0;
}
