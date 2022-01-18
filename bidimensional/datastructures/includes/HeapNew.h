//
// Created by pedro on 18.08.21.
//

#ifndef BIDIJKSTRA_HEAPNEW_H
#define BIDIJKSTRA_HEAPNEW_H

#include "Label.h"

class BinaryHeap
{

public:

    explicit BinaryHeap(CostType fmin)
            : lastElementIndex(0), elts_(0) {
        this->elts_.resize(1);
    }

    void decrease_key(CostType c1, Label* val) {
        assert(val->priority < lastElementIndex);
        heapify_up(val->priority);
    }

    void push(Label* val) {
        if(lastElementIndex + 1 > elts_.size()) {
            elts_.resize(elts_.size()*2);
        }
        size_t priority = lastElementIndex;
        elts_[priority] = val;

        val->priority = priority;
        lastElementIndex++;
        heapify_up(priority);
        val->inQueue = true;
    }

    Label* pop() {
        assert(this->size() != 0);

        Label* ans = elts_[0];
        lastElementIndex--;

        if(lastElementIndex > 0)
        {
            elts_[0] = elts_[lastElementIndex];
            elts_[0]->priority = 0;
            heapify_down(0);
        }
        ans->inQueue=false;

        return ans;
    }


    // @return true if the priority of the element is
    // otherwise
    inline bool
    contains(Label* n)
    {
        size_t priority = n->priority;
        if(priority < lastElementIndex && &*n == &*elts_[priority] )
        {
            return true;
        }
        return false;
    }



    inline size_t
    size() const
    {
        return lastElementIndex;
    }

private:
    size_t lastElementIndex;
    std::vector<Label*> elts_;
    LexComparison cmp_;

    // reorders the subpqueue containing elts_[index]
    void
    heapify_up(size_t index)
    {
        assert(index < lastElementIndex);
        while(index > 0)
        {
            size_t parent = (index-1) >> 1; //Shift right dividing by 2

            if(cmp_(elts_[index], elts_[parent]))
            {
                swap(parent, index);
                index = parent;
            }
            else { break; }
        }
    }


    // reorders the subpqueue under elts_[index]
    void
    heapify_down(size_t index)
    {
        size_t first_leaf_index = lastElementIndex >> 1;
        while(index < first_leaf_index)
        {
            // find smallest (or largest, depending on heap type) child
            size_t child1 = (index<<1)+1;
            size_t child2 = (index<<1)+2;
            size_t which = child1;
            if((child2 < lastElementIndex) &&
               cmp_(elts_[child2], elts_[child1]))
            { which = child2; }

            // swap child with parent if necessary
            if(cmp_(elts_[which], elts_[index]))
            {
                swap(index, which);
                index = which;
            }
            else { break; }
        }
    }

    // swap the positions of two nodes in the underlying array
    inline void
    swap(size_t index1, size_t index2)
    {
        assert(index1 < lastElementIndex && index2 < lastElementIndex);

        Label* tmp1 = elts_[index1];
        elts_[index1] = elts_[index2];
        elts_[index2] = tmp1;
        elts_[index1]->priority = index1;
        elts_[index2]->priority = index2;

    }
};

#endif //BIDIJKSTRA_HEAPNEW_H
