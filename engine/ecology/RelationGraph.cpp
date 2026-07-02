#include "RelationGraph.hpp"

#include <algorithm>

void RelationGraph::addLineage(int parentId, int childId)
{
    lineage_.push_back(Relation{parentId, childId, RelationType::Lineage});
}

void RelationGraph::addCompetition(int a, int b)
{
    if (a > b) {
        std::swap(a, b);   // symétrique : une seule arête, ordonnée
    }
    competition_.push_back(Relation{a, b, RelationType::Competition});
}

void RelationGraph::clearCompetition()
{
    competition_.clear();
}

void RelationGraph::removeOrganism(int id)
{
    const auto touches = [id](const Relation& r) {
        return r.from == id || r.to == id;
    };
    lineage_.erase(std::remove_if(lineage_.begin(), lineage_.end(), touches),
                   lineage_.end());
    competition_.erase(std::remove_if(competition_.begin(), competition_.end(), touches),
                       competition_.end());
}

const std::vector<Relation>& RelationGraph::lineage() const
{
    return lineage_;
}

const std::vector<Relation>& RelationGraph::competition() const
{
    return competition_;
}

std::vector<Relation> RelationGraph::all() const
{
    std::vector<Relation> result = lineage_;
    result.insert(result.end(), competition_.begin(), competition_.end());
    return result;
}
