#ifndef BUCKETS_BDA_
#define BUCKETS_BDA_

#include "typedefs.h"
#include "Label_Buckets.h"

class BucketsQueue
{
    //typedef std::list<LabelBuckets*> BucketContent;
    //typedef std::unique_ptr<BucketContent> BucketContentPtr;
public:

    /**
     * One bucket per c1 value of the considered labels. However, since buckets are emptied from left to right, many
     * empty buckets could remain before the first one that actually contains some label. That's why at some point,
     * the first non-empty bucket and all bucket behind it might be shifted in memory to the beginning of the 'buckets' vector.
     *
     * To still be able to compute the position of the bucket for a label with c1 costs as its first cost component, we
     * need
     *     - the 'initialBucketValue' that specifies the first c1 value that was inserted into the queue,
     *     - the 'currentMinBucket' that specifies the c1 value of the elements in the current first bucket in the 'buckets' vector,
     *     - the 'currentBucket' that gives the index in the current 'buckets' vector of the bucket containinig laabels whose first cost component equals c1,
     *     - the 'cumulatedShifts' that gives the overall number of shifts done so far.
     * @param firstC1Value
     */
    explicit BucketsQueue(CostType firstC1Value):
            elementsCount(0),
            currentBucket(0),
            initialBucketValue{firstC1Value},
            currentMinBucket(firstC1Value),
            buckets(0),
            cumulatedShifts{0} {}

    ~BucketsQueue() {
        this->clear();
    }

    void clear() {
        elementsCount = 0;
        buckets.clear();
        currentBucket = 0;
    }

    inline size_t getBucketIndex(LabelBuckets* l) const {
        return (l->c1 - currentMinBucket) + currentBucket;
    }

    void push (LabelBuckets* lb) {
        size_t index = getBucketIndex(lb);
        if (index >= buckets.size()) {
            resize(index * 1.5 + 1);
            index -= currentBucket;
            currentBucket = 0;
        }

        LabelBuckets* firstElementInBucket = this->buckets[index];
        lb->next = firstElementInBucket;
        if (firstElementInBucket != nullptr) {
            firstElementInBucket->prev = lb;
        }
        this->buckets[index] = lb;
        lb->inQueue = true;
        elementsCount++;
    }

    inline LabelBuckets* pop() {
        LabelBuckets* returnLabel{nullptr};
        if(!this->empty()) {
            while(this->buckets[this->currentBucket] == nullptr) {
                this->currentBucket++;
                this->currentMinBucket++;
            }
            //assert(this->countUntilIndex(this->currentBucket) == 0);
            returnLabel = this->buckets[currentBucket];
            assert(returnLabel != nullptr);
            returnLabel->inQueue = false;
            this->buckets[currentBucket] = returnLabel->next;
            if (this->buckets[currentBucket] != nullptr) {
                this->buckets[currentBucket]->prev = nullptr;
            }
            --this->elementsCount;
        }
        return returnLabel;
    }

    inline bool empty() const {
        return this->elementsCount == 0;
    }

    inline bool size() const {
        return this->elementsCount;
    }

    void resize(size_t nSize) {
        size_t oldSize = this->bucketsCount();
        //First, we move all lists to the beginning of the vector to reuse the buckets that are already empty.
        if (currentBucket != 0) {
            this->cumulatedShifts += this->currentBucket;
            for (size_t i = this->currentBucket, j = 0; i < oldSize; ++i,++j) {
                this->buckets[j] = this->buckets[i];
                this->buckets[i] = nullptr;
            }
        }
        buckets.resize(nSize, nullptr);
    }

    inline void decreaseKey(CostType oldBucket, LabelBuckets* updatedL) {
        assert(updatedL->inQueue);
        //assert(this->elementsCount == this->manualCount());
        LabelBuckets* prev = updatedL->prev;
        LabelBuckets* next = updatedL->next;
        if (prev != nullptr) {
            prev->next = next;
        }
        else {//Element is in first position!
            size_t indexWithoutShifts{oldBucket - this->initialBucketValue};
            size_t indexAfterShifts{indexWithoutShifts - this->cumulatedShifts};
            this->buckets[indexAfterShifts] = updatedL->next;
        }
        if (next != nullptr) {
            next->prev = prev;
        }
        size_t newBucketIndex{this->getBucketIndex(updatedL)};
        LabelBuckets* headOfNewBucket{this->buckets[newBucketIndex]};
        this->buckets[newBucketIndex] = updatedL;
        updatedL->next = headOfNewBucket;
        updatedL->prev = nullptr;
        if (headOfNewBucket != nullptr) {
            headOfNewBucket->prev = updatedL;
        }
        //assert(this->elementsCount == this->manualCount());
    }

    inline size_t bucketsCount() const {
        return this->buckets.size();
    }

private:
    CostType elementsCount;
    CostType currentBucket;
    const CostType initialBucketValue;
    CostType currentMinBucket;
    std::vector<LabelBuckets*> buckets;
    size_t cumulatedShifts;

};

#endif