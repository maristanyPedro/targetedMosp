#ifndef PERMANENTS_H_
#define PERMANENTS_H_

#include <cassert>
#include "../../datastructures/includes/typedefs.h"

typedef std::pair<size_t , uint16_t> PredLabel;
constexpr uint16_t labelsPerRow = 1000;

class Permanents {
public:
    Permanents():
        elements(1)
    {}

    inline void addElement(size_t predIndex, ArcId predArcId) {
        assert(currentIndex < labelsPerRow);
        if (currentIndex < labelsPerRow) {
            elements.back()[currentIndex].first = predIndex;
            elements.back()[currentIndex].second = predArcId;
            this->increaseIndex();
        }
    }

    inline size_t getCurrentIndex() const {
        assert(currentIndex < labelsPerRow);
        return (this->elements.size()-1)*labelsPerRow + this->currentIndex;
    }

    inline const PredLabel& getElement(size_t index) const {
        size_t rowIndex = index/labelsPerRow;
        size_t indexInRow = index%labelsPerRow;
        return this->elements[rowIndex][indexInRow];
    }

    inline size_t size() const {
        return (this->elements.size()-1)*labelsPerRow + currentIndex;
    }

private:
    void increaseIndex() {
        if (currentIndex + 1 == labelsPerRow) {
            this->elements.emplace_back(std::array<PredLabel, labelsPerRow>());
            currentIndex = 0;
        }
        else {
            ++currentIndex;
        }
    }

    std::vector<std::array<PredLabel, labelsPerRow>> elements;
    uint16_t currentIndex{0};
};

#endif