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
    CostType latestEfficientSecondComponent{MAX_COST};
    Node n{INVALID_NODE};
    Info<CostType> dominanceBound{MAX_COST, MAX_COST};
    Info<uint32_t> priority; ///< for heap operations!
    Info<bool> reached{false, false};

    void updatePreprocessingInfo(bool costComponent, CostType cost_1, CostType cost_2, const Arc* predArc) {
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