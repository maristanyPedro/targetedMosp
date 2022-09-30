//
// Created by pedro on 18.08.21.
//

#ifndef BIDIJKSTRA_HEAPNEW_H
#define BIDIJKSTRA_HEAPNEW_H

#include "Label.h"

class BinaryHeap {
public:

    explicit BinaryHeap(CostType fmin)
            : lastElementIndex(0), heapElements(0) {
        this->heapElements.resize(1);
    }

    //The unused parameter is only there to ensure that the function has the same signature as Buckets::decreaseKey.
    void decreaseKey(CostType c1, Label *val) {
        assert(val->priority < lastElementIndex);
        up(val->priority);
    }

    void push(Label *val) {
        if (lastElementIndex + 1 > heapElements.size()) {
            heapElements.resize(heapElements.size() * 2);
        }
        heapElements[lastElementIndex] = val;

        val->priority = lastElementIndex;
        size_t temp = lastElementIndex;
        lastElementIndex++;
        up(temp);
        val->inQueue = true;
    }

    Label *pop() {
        assert(this->size() != 0);

        Label *ans = heapElements[0];
        lastElementIndex--;

        if (lastElementIndex > 0) {
            heapElements[0] = heapElements[lastElementIndex];
            heapElements[0]->priority = 0;
            down(0);
        }
        ans->inQueue = false;

        return ans;
    }

    inline bool contains(Label *n) {
        size_t priority = n->priority;
        if (priority < lastElementIndex && &*n == &*heapElements[priority]) {
            return true;
        }
        return false;
    }

    inline size_t
    size() const {
        return lastElementIndex;
    }

private:
    size_t lastElementIndex;
    std::vector<Label *> heapElements;
    LexComparison comparator;

    void up(size_t index) {
        assert(index < lastElementIndex);
        while (index > 0) {
            size_t parent = (index - 1) >> 1; //Shift right dividing by 2

            if (comparator(heapElements[index], heapElements[parent])) {
                swap(parent, index);
                index = parent;
            } else { break; }
        }
    }

    void down(size_t index) {
        size_t first_leaf_index = lastElementIndex >> 1;
        while (index < first_leaf_index) {
            // find smallest (or largest, depending on heap type) child
            size_t child1 = (index << 1) + 1;
            size_t child2 = (index << 1) + 2;
            size_t which = child1;
            if ((child2 < lastElementIndex) &&
                comparator(heapElements[child2], heapElements[child1])) { which = child2; }

            // swap child with parent if necessary
            if (comparator(heapElements[which], heapElements[index])) {
                swap(index, which);
                index = which;
            } else { break; }
        }
    }

    inline void swap(size_t index1, size_t index2) {
        Label *tmp1 = heapElements[index1];
        heapElements[index1] = heapElements[index2];
        heapElements[index2] = tmp1;
        heapElements[index1]->priority = index1;
        heapElements[index2]->priority = index2;

    }
};

#endif
