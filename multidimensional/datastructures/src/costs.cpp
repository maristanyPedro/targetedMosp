#include "../includes/costs.h"
#include <iostream>

bool Costs::lexSmaller12(const Costs &other) const {
    if (this->c1 < other.c1) {
        return true;
    } else if (this->c1 == other.c1) {
        return this->c2 < other.c2;
    }
    return false;
}

bool Costs::lexSmaller21(const Costs &other) const {
    if (this->c2 < other.c2) {
        return true;
    } else if (this->c2 == other.c2) {
        return this->c1 < other.c1;
    }
    return false;
}

void Costs::print() const {
    printf("(%d. %d) \n", this->c1, this->c2);
}
