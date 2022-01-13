#ifndef NODE_INFO_H
#define NODE_INFO_H

#include <algorithm>
#include <array>
#include <vector>
#include "costs.h"

struct Arc;

typedef std::vector<CostArray> PermanentCosts;
/**
 * Bundles all relevant information related to a node in the graph.
 */
struct NodeInfo {
    //A dxd matrix storing the results of the d preprocessing lexicographic searches.
    Info<CostArray> preprocessingResult;
    //Computed during preprocessing. This is a lowerbound from the current node to the target node w.r.t. every cost dimension.
    CostArray potential{MAX_COST, MAX_COST, MAX_COST};
    //Current node for which 'this' stored information.
    Node n{INVALID_NODE};
    Info<uint32_t> priority; ///< for heap operations in preprocessing!
    //Used in preprocessing to flag if the node has been reached by the lex searches. The ith entry of the vector specifies
    //whether the node has been reached by a lex. search in which the ith cost component was the main optimization component.
    Info<bool> reached{false, false, false};

    //Pareto front of s-v-costs, if v is the current node.
    PermanentCosts efficientCosts;
    //The following two members allow path reconstructions. Storing paths like this is motivated by the code used for the
    //publication DOI: 10.4230/LIPIcs.ESA.2021.3 by Ahmadi et al.
    std::vector<u_int16_t> pathIds;
    std::vector<u_int16_t> incomingEfficientArcs;

    NodeInfo() {
        for (size_t i = 0; i < DIM; ++i) {
            for (size_t j = 0; j < DIM; ++j) {
                preprocessingResult[i][j] = MAX_COST;
            }
        }
    }

    inline void updatePreprocessingInfo(const DimensionsVector& dimOrdering, CostArray c) {
        Dimension mainOptIndex = dimOrdering[0];
        CostArray& result = preprocessingResult[mainOptIndex];
        result = c;
    }

    inline uint32_t get_priority(Dimension index) const{
        return priority[index];
    }

    inline void set_priority(Dimension index, uint32_t prio) {
        priority[index] = prio;
    }

    inline u_int16_t latestPathIndex() const {
        return pathIds.size();
    }
};

inline bool dominatedAtFront(const PermanentCosts& pc, const CostArray& c) {
    return std::any_of(pc.begin(), pc.end(), [&c](const CostArray& it)->bool{return dominates(it, c);});
}

//Comparator used for lex-min searches during preprocessing.
struct PotentialComparison {
    PotentialComparison() = delete;
    explicit PotentialComparison(const DimensionsVector& d):
        dimOrdering{d} {}
    inline bool operator() (const NodeInfo* lhs, const NodeInfo* rhs) const {
        const CostArray& lh{lhs->preprocessingResult[this->dimOrdering[0]]};
        const CostArray& rh{rhs->preprocessingResult[this->dimOrdering[0]]};
        return lexSmaller(lh, rh);
    }
    const DimensionsVector& dimOrdering;
};

#endif //NODE_INFO_H
