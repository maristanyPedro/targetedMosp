#ifndef NODE_INFO_H
#define NODE_INFO_H

#include <array>
#include <vector>
#include "Label_Buckets.h"

struct Arc;

struct NodeInfo {
    std::vector<MinLabel> permanentLabels;
    Info<const Arc*> preprocessingParent{nullptr, nullptr};
    Info<CostType> minCosts{MAX_COST, MAX_COST};
    Info<CostType> potential{MAX_COST, MAX_COST};
    Info<CostType> dominanceBound{MAX_COST, MAX_COST};
    Info<uint32_t> priority; ///< for heap operations during preprocessing searches.
    CostType latestEfficientSecondComponent{MAX_COST};
    Node n{INVALID_NODE};
    Info<bool> reached{false, false};

    void updatePreprocessingInfo(bool costComponent, CostType cost_1, CostType cost_2, const Arc* predArc) {
        //The result of the substraction is the price of the currently shortest known path from the target of the
        //preprocessing search to the current node. cost_1 is the cost of a newly explored path from the source of the
        //preprocessing search to the current node. Hence, the addition of the substraction's result and cost_1 gives a
        //lower bound on the costs of an s-t-path that the preprocessing search might find.
        this->minCosts[costComponent] = this->minCosts[costComponent] - this->potential[costComponent] + cost_1;
        this->potential[costComponent] = cost_1;
        this->dominanceBound[costComponent] = cost_2;
        this->preprocessingParent[costComponent] = predArc;
    }

    inline uint32_t get_priority(bool index) const{
        return priority[index];
    }

    inline void set_priority(bool index, uint32_t prio) {
        priority[index] = prio;
    }

    inline u_int16_t latestPathIndex() const {
        return permanentLabels.size();
    }
};

struct PotentialComparison {
    inline bool operator() (const NodeInfo* lhs, const NodeInfo* rhs, bool index) const {
        return lhs->minCosts[index] < rhs->minCosts[index];
    }
};

#endif //NODE_INFO_H