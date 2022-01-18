#ifndef LABEL_BUCKETS_H_
#define LABEL_BUCKETS_H_

#include "typedefs.h"

struct LabelBuckets {

    inline void update(CostType c1, CostType c2, Node n , u_int16_t predArc, uint16_t pathId) {
        //assert(pathId <= std::numeric_limits<u_int16_t>::max());
        this->c1 = c1;
        this->c2 = c2;
        this->n = n;
        this->predArcId = predArc;
        this->pathId = pathId;
    }

    LabelBuckets* prev = nullptr;
    LabelBuckets* next = nullptr;
    CostType c1 = MAX_COST, c2 = MAX_COST;
    Node n = INVALID_NODE;
    uint16_t predArcId = std::numeric_limits<uint16_t>::max();
    uint16_t pathId = std::numeric_limits<uint16_t>::max();
    bool inQueue = false;
};

struct MinLabel {
    MinLabel(CostType first, CostType second, uint16_t arc, uint16_t path):
        c1{first}, c2{second}, predArcId{arc}, pathId{path} {};

    MinLabel() = delete;
    CostType c1 = MAX_COST, c2 = MAX_COST;
    uint16_t predArcId = std::numeric_limits<uint16_t>::max();
    uint16_t pathId = std::numeric_limits<uint16_t>::max();
};

#endif