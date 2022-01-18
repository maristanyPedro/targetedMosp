#ifndef BUCKETS_BDA_
#define BUCKETS_BDA_

#include "typedefs.h"
#include "Label_Buckets.h"

class Buckets_BDA
{
    //typedef std::list<LabelBuckets*> BucketContent;
    //typedef std::unique_ptr<BucketContent> BucketContentPtr;
public:

    explicit Buckets_BDA(CostType fmin):
            elementsCount(0),
            currentBucket(0),
            originalF_min{fmin},
            f_min(fmin),
            buckets(0),
            cumulatedShifts{0} {}

    ~Buckets_BDA() {
        this->clear();
    }

    void clear() {
        elementsCount = 0;
        buckets.clear();
        currentBucket = 0;
    }

    inline size_t getBucketIndex(LabelBuckets* l) const {
        return (l->c1 - f_min) + currentBucket;
    }


    // add a new element to the pqueue
    void push (LabelBuckets* lb) {
        //assert(this->elementsCount == this->manualCount());
        unsigned int index = getBucketIndex(lb);
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
        //assert(this->elementsCount == this->manualCount());
        return;
    }

    inline LabelBuckets* pop() {
        LabelBuckets* returnLabel{nullptr};
        //assert(this->elementsCount == this->manualCount());
        if(!this->empty()) {
            while(this->buckets[this->currentBucket] == nullptr) {
                this->currentBucket++;
                this->f_min++;
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
        //assert(this->elementsCount == this->manualCount());
        return returnLabel;
    }

    inline bool empty() const {
        return this->elementsCount == 0;
    }

    inline bool size() const {
        return this->elementsCount;
    }

    size_t countUntilIndex(size_t index) const {
        assert(index <= this->bucketsCount());
        size_t counter = 0;
        for (size_t i = 0; i < index; ++i) {
            LabelBuckets* bucket = this->buckets[i];
            if (bucket != nullptr) {
                LabelBuckets* element = this->buckets[i];
                while (element != nullptr) {
                    ++counter;
                    element = element->next;
                }
            }
        }
        return counter;
    }

    size_t manualCount() const {
        return countUntilIndex(this->bucketsCount());
    }

    //FOR VECTOR IMPL.
    void resize(size_t nSize) {
        size_t oldSize = this->bucketsCount();
        //printf("Start resize from %lu to %lu. CurrentBucket: %lu\n", oldSize, nSize, currentBucket);

        //First, we move all lists to the beginning of the vector to reuse the buckets that are already empty.
        if (currentBucket != 0) {
            this->cumulatedShifts += this->currentBucket;
            for (size_t i = this->currentBucket, j = 0; i < oldSize; ++i,++j) {
                this->buckets[j] = this->buckets[i];
                this->buckets[i] = nullptr;
            }
        }

        //Now, we create room for new buckets. The new buckets initially contain a nullptr.
        buckets.resize(nSize, nullptr);
    }

    inline void decrease_key(CostType oldBucket, LabelBuckets* updatedL) {
        assert(updatedL->inQueue);
        //assert(this->elementsCount == this->manualCount());
        LabelBuckets* prev = updatedL->prev;
        LabelBuckets* next = updatedL->next;
        if (prev != nullptr) {
            prev->next = next;
        }
        else {//Element is in first position!
            size_t indexWithoutShifts{oldBucket - this->originalF_min};
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

//    size_t
//    mem()
//    {
//        return bucketsCount*sizeof (L*)
//               + sizeof(*this);
//    }

private:
    unsigned int elementsCount;
    unsigned int currentBucket;
    const CostType originalF_min;
    CostType f_min;
    std::vector<LabelBuckets*> buckets;
    size_t cumulatedShifts;

};

#endif