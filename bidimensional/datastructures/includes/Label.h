#ifndef LABEL_H_
#define LABEL_H_

#include "typedefs.h"

struct Label {

    inline void update(CostType c1, CostType c2, Node n , u_int16_t predArc, uint16_t pathId) {
        //assert(pathId <= std::numeric_limits<u_int16_t>::max());
        this->c1 = c1;
        this->c2 = c2;
        this->n = n;
        this->predArcId = predArc;
        this->pathId = pathId;
    }

    uint32_t priority; ///< for heap operations.
    CostType c1 = MAX_COST, c2 = MAX_COST;
    Node n = INVALID_NODE;
    uint16_t predArcId = std::numeric_limits<uint16_t>::max();
    uint16_t pathId = std::numeric_limits<uint16_t>::max();
    bool inQueue = false;
};

struct LexComparison {
    inline bool operator() (const Label* lhs, const Label* rhs) const {
        return lhs->c1 < rhs->c1 || (lhs->c1 == rhs->c1 && lhs->c2 < rhs->c2);
    }
};

#endif