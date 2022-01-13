//
// Created by pedro on 18.08.21.
//

#ifndef MOSP_HEAP_H
#define MOSP_HEAP_H

#include <cassert>
#include "Label.h"

class BinaryHeapMosp {

public:

    explicit BinaryHeapMosp(size_t size)
            : lastElementIndex(0), heapElements(0) {
        this->heapElements.resize(size);
    }

    void decreaseKey(Label *val) {
        assert(val->priority < lastElementIndex);
        heapifyUp(val->priority);
    }

    void push(Label *val) {
        if (lastElementIndex + 1 > heapElements.size()) {
            heapElements.resize(heapElements.size() * 2);
        }
        size_t priority = lastElementIndex;
        heapElements[priority] = val;

        val->priority = priority;
        lastElementIndex++;
        heapifyUp(priority);
    }

    inline Label *pop() {
        Label *ans = heapElements[0];
        lastElementIndex--;

        if (lastElementIndex > 0) {
            heapElements[0] = heapElements[lastElementIndex];
            heapElements[0]->priority = 0;
            heapifyDown(0);
        }
        return ans;
    }

    inline bool contains(Label *n) {
        size_t priority = n->priority;
        if (priority < lastElementIndex && n == heapElements[priority]) {
            return true;
        }
        return false;
    }

    inline size_t size() const {
        return lastElementIndex;
    }

private:
    size_t lastElementIndex;
    std::vector<Label *> heapElements;
    LexComparison comparator;

    void heapifyUp(size_t index) {
        assert(index < lastElementIndex);
        while (index > 0) {
            size_t parent = (index - 1) >> 1; //Shift right dividing by 2

            if (comparator(heapElements[index], heapElements[parent])) {
                swap(parent, index);
                index = parent;
            } else {
                break;
            }
        }
    }

    void heapifyDown(size_t index) {
        size_t first_leaf_index = lastElementIndex >> 1;
        while (index < first_leaf_index) {
            size_t child1 = (index << 1) + 1;
            size_t child2 = (index << 1) + 2;
            size_t swapCandidate = child1;
            if ((child2 < lastElementIndex) && comparator(heapElements[child2], heapElements[child1])) {
                swapCandidate = child2;
            }
            if (comparator(heapElements[swapCandidate], heapElements[index])) {
                swap(index, swapCandidate);
                index = swapCandidate;
            } else {
                break;
            }
        }
    }

    inline void swap(size_t id1, size_t id2) {
        assert(id1 < lastElementIndex && id2 < lastElementIndex);
        Label *temp = heapElements[id1];
        heapElements[id1] = heapElements[id2];
        heapElements[id2] = temp;
        heapElements[id1]->priority = id1;
        heapElements[id2]->priority = id2;

    }
};

#endif
