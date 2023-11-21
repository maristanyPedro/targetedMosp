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
            : lastElementIndex(0), heapElements(0), elemCount{0}, maxElemCount{0} {
        this->heapElements.resize(size);
    }

    void decreaseKey(Label *lOld, Label* lNew) {
        ++decreaseKeyCounter;
        lNew->priority = lOld->priority;
        heapElements[lNew->priority] = lNew;
        up(lNew->priority);
    }

    void push(Label *lNew) {
        if (lastElementIndex + 1 > heapElements.size()) {
            heapElements.resize(heapElements.size() * 2);
        }
        size_t priority = lastElementIndex;
        heapElements[priority] = lNew;

        lNew->priority = priority;
        lastElementIndex++;
        up(priority);
        ++elemCount;
        if (elemCount > maxElemCount) {
            maxElemCount = elemCount;
        }
    }

    inline Label *pop() {
        Label *minElement = heapElements[0];
        lastElementIndex--;

        if (lastElementIndex > 0) {
            heapElements[0] = heapElements[lastElementIndex];
            heapElements[0]->priority = 0;
            down(0);
        }
        --elemCount;
        return minElement;
    }

    inline size_t size() const {
        return lastElementIndex;
    }

    size_t maxSize() const {
        return this->maxElemCount;
    }

private:
    size_t lastElementIndex;
    std::vector<Label *> heapElements;
    LexComparison comparator;
    size_t elemCount;
    size_t maxElemCount;
public:
    size_t decreaseKeyCounter{0};
private:

    void up(size_t index) {
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

    void down(size_t index) {
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
