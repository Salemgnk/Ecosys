#ifndef RELATIONGRAPH_HPP
    #define RELATIONGRAPH_HPP
    #include <vector>

// Les relations entre organismes, identifiés par leur id stable.
// Deux natures en v1 :
//   Lineage     — parent -> enfant, née d'une reproduction (durable)
//   Competition — deux organismes se disputent une même zone (recalculée
//                 à chaque tick, symétrique : stockée une fois, from < to)
enum class RelationType { Lineage, Competition };

struct Relation {
    int from;
    int to;
    RelationType type;
};

class RelationGraph {
    private:
        std::vector<Relation> lineage_;
        std::vector<Relation> competition_;

    public:
        void addLineage(int parentId, int childId);
        void addCompetition(int a, int b);
        void clearCompetition();          // recalcul à chaque tick
        void removeOrganism(int id);      // un mort quitte le graphe

        const std::vector<Relation>& lineage() const;
        const std::vector<Relation>& competition() const;
        std::vector<Relation> all() const;   // concaténé, pour les vues
};

#endif
