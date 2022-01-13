#ifndef LABEL_H_
#define LABEL_H_

#include "typedefs.h"

struct Label {

    inline void update(const CostArray& cNew, Node n , ArcId predArc, uint16_t pathId) {
        //assert(pathId <= std::numeric_limits<u_int16_t>::max());
        this->c = cNew;
        this->n = n;
        this->predArcId = predArc;
        this->pathId = pathId;
    }

    CostArray c{MAX_COST, MAX_COST, MAX_COST};
    Node n = INVALID_NODE;
    ArcId predArcId = INVALID_ARC;
    uint16_t pathId = std::numeric_limits<uint16_t>::max();
    uint32_t priority; ///< for heap operations.
};

struct LexComparison {
    inline bool operator() (const Label* lhs, const Label* rhs) const {
        return lexSmaller(lhs->c, rhs->c);
    }
};

#endif