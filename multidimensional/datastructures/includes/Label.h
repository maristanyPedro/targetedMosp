#ifndef LABEL_H_
#define LABEL_H_

#include "typedefs.h"

struct Label {

    Label() = default;

    Label(CostArray& c, Node n, ArcId a, uint16_t p):
        c{c}, n{n}, predArcId{a}, priority{0}, pathId{p} {}

    inline void update(const CostArray& cNew, Node n , ArcId predArc, uint16_t pathId, uint16_t targetFrontSize) {
        //assert(pathId <= std::numeric_limits<u_int16_t>::max());
        this->c = cNew;
        this->n = n;
        this->predArcId = predArc;
        this->pathId = pathId;
        this->knownTargetElements = targetFrontSize;
        this->nclChecked = false;
    }

    Label& operator=(const Label& other) {
//        exit(1);
        this->c = other.c;
        this->n = other.n;
        this->predArcId = other.predArcId;
        this->pathId = other.pathId;
        return *this;
    }

    CostArray c{MAX_COST, MAX_COST, MAX_COST};
    Node n = INVALID_NODE;
    ArcId predArcId = INVALID_ARC;
    uint32_t priority; ///< for heap operations.
    uint16_t pathId = std::numeric_limits<uint16_t>::max();
    uint16_t knownTargetElements{0};
    Label* next{nullptr};
    bool nclChecked{false};
};

struct LexComparison {
    inline bool operator() (const Label* lhs, const Label* rhs) const {
        return lexSmaller(lhs->c, rhs->c);
    }
};

class List {
public:
    Label* first{nullptr};
    Label* last{nullptr};
    size_t size{0};

    inline void push_back(Label* ec) {
        if (this->empty()) {
            this->first = ec;
            this->last = ec;
            ec->next = nullptr;
        }
        else {
            this->last->next = ec;
            this->last = ec;
            ec->next = nullptr;
        }
        ++size;
    }

    inline void push_front(Label* l) {
        if (this->empty()) {
            this->first = l;
            this->last = l;
            l->next = nullptr;
        }
        else {
            l->next = this->first;
            this->first = l;
        }
        ++size;
    }

    inline Label* pop_front() {
        if (this->empty()) {
            return nullptr;
        }
        else if (this->size == 1) {
            this->first = nullptr;
            this->last = nullptr;
            --size;
            return nullptr;
        }
        else {
            Label* front = this->first;
            this->first = front->next;
            --size;
            return this->first;
        }
    }

    inline bool empty() const {
        return this->size == 0;
    }
};

#endif