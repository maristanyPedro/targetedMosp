#ifndef NODECONTAINER_H
#define NODECONTAINER_H

#include <algorithm>
#include <vector>
#include <../../graph/includes/graph.h>
#include <../../datastructures/includes/typedefs.h>

/**
 * Wraps a vector of NodeInfos. For each node in the graph, the vector has one entry. The entry containing the NodeInfo
 * for node with id n is in the vector position n.
 */
 template <typename NodeInfoType>
struct NodeInfoContainer {

    explicit NodeInfoContainer(const Graph& G):
        nodeInfos(G.nodesCount) {
        this->initialize();
    }

    inline NodeInfoType& getInfo(const Node n) {
        return this->nodeInfos[n];
    }

    std::vector<NodeInfoType> nodeInfos;

private:
    void initialize() {
        for (Node n = 0; n < this->nodeInfos.size(); ++n) {
            this->nodeInfos[n].n = n;
        }
    }
};

//Comparator used for lex-min searches during preprocessing.
template <typename LabelType>
struct PotentialComparison {
    PotentialComparison() = delete;
    explicit PotentialComparison(const DimensionsVector& d):
            dimOrdering{d} {}
    inline bool operator() (const LabelType* lhs, const LabelType* rhs) const {
        const CostArray& lh{lhs->preprocessingResult[this->dimOrdering[0]]};
        const CostArray& rh{rhs->preprocessingResult[this->dimOrdering[0]]};
        return lexSmaller(lh, rh);
    }
    const DimensionsVector& dimOrdering;
};

#endif