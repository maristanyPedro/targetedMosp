//
// Created by pedro on 10.08.21.
//

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <array>
#include <cstddef> //For size_t.
#include <limits>
#include <list>
#include <cstdint>
#include <cinttypes>
#include <vector>

typedef uint32_t Node;
typedef uint16_t NeighborhoodSize;
typedef uint32_t ArcId;
typedef uint32_t CostType;
typedef u_short Dimension;

//#define GENERATE_MISSING_COST_COMPONENTS_UNIF_RANDOM
constexpr Dimension DIM = 3;

constexpr Node INVALID_NODE = std::numeric_limits<Node>::max();
constexpr NeighborhoodSize MAX_DEGREE = std::numeric_limits<NeighborhoodSize>::max();
constexpr ArcId INVALID_ARC = std::numeric_limits<ArcId>::max();
constexpr CostType MAX_COST = std::numeric_limits<CostType>::max();
constexpr uint16_t MAX_PATH = std::numeric_limits<uint16_t>::max();

template <typename T>
using Info = std::array<T, DIM>;
typedef Info<CostType> CostArray;

inline CostArray generate(CostType c = MAX_COST) {
    CostArray newca;
    newca.fill(c);
    return newca;
}

typedef Info<Dimension> DimensionsVector;

template <typename T>
using DimReductedInfo = std::array<T, DIM-1>;
typedef DimReductedInfo<CostType> TruncatedCosts;
typedef std::list<TruncatedCosts> TruncatedFront;

inline bool lexSmaller(const CostArray& lh, const CostArray& rh) {
    for (size_t i = 0; i < DIM; ++i) {
        if (lh[i] < rh[i]) {
            return true;
        }
        else if (lh[i] == rh[i]) {
            continue;
        }
        else {
            return false;
        }
    }
    //Happens if both arrays coincide.
    return false;
}

inline bool lexSmaller(const TruncatedCosts& lh, const TruncatedCosts& rh) {
    for (size_t i = 0; i < DIM-1; ++i) {
        if (lh[i] < rh[i]) {
            return true;
        }
        else if (lh[i] == rh[i]) {
            continue;
        }
        else {
            return false;
        }
    }
    //Happens if both arrays coincide.
    return false;
}

inline TruncatedCosts truncate(const CostArray& c) {
    TruncatedCosts tc;
    for (Dimension i = 1; i < DIM; ++i) {
        tc[i-1] = c[i];
    }
    return tc;
}

inline bool tc_dominates(const TruncatedCosts& lhs, const TruncatedCosts& rhs) {
    for (size_t i = 0; i < DIM-1; ++i) {
        if (lhs[i] > rhs[i]) {
            return false;
        }
    }
    return true;
//    return lhs[0] <= rhs[0] && lhs[1] <= rhs[1] && lhs[2] <= rhs[2];
}

inline bool tc_dominates(const TruncatedCosts& lhs, const CostArray& rhs) {
    for (size_t i = 0; i < DIM-1; ++i) {
        if (lhs[i] > rhs[i+1]) {
            return false;
        }
    }
    return true;
//    return lhs[0] <= rhs[0] && lhs[1] <= rhs[1] && lhs[2] <= rhs[2];
}

/**
 * Updates the front of truncated costs with the new cost vector c. This functions is heavy in assumptions. front must be
 * sorted lexicographically to guarantee that c is inserted at the correct position. Then, front stays sorted. Moreover,
 * c is guaranteed to be the cost vector of an efficient vector and thus, its truncated cost vector is guaranteed to not
 * be dominated by any vector in front. Thus, the update of front only need to find the correct insertion position for
 * c and remove elements from front that are dominated by the truncated vector of c.
 * @param front
 * @param c
 */
inline void truncatedInsertionBackward(TruncatedFront& front, const CostArray& c) {
    TruncatedCosts tc = truncate(c);
    if (front.empty()) {
        front.push_back(tc);
        return;
    }
    auto it = front.rbegin();
    while (it != front.rend() && lexSmaller(tc, *it)) {
        if (tc_dominates(tc, *it)) {
            front.erase(std::next(it).base());
        }
        else {
            std::advance(it, 1);
        }
    }
    front.emplace(it.base()--, tc);
}


inline void truncatedInsertion3d(TruncatedFront& front, const CostArray& c) {
    TruncatedCosts tc = truncate(c);
    if (front.empty()) {
        front.push_back(tc);
        return;
    }
    auto it = front.begin();
    while (it != front.end() && lexSmaller(*it, tc)) {
        ++it;
    }
    it = front.emplace(it, tc);
    ++it;
    while (it != front.end()) {
        if (tc_dominates(tc, *it)) {
            it = front.erase(it);
        } else {
            //++it;
            break;
        }
    }
}

inline void truncatedInsertion(TruncatedFront& front, const CostArray& c) {
    DIM == 3? truncatedInsertion3d(front, c) : truncatedInsertionBackward(front, c);
}

inline std::pair<bool, size_t> lexSmallerOrEquivCounter(const TruncatedCosts& lh, const CostArray& rh) {
    size_t counter = 0;
    for (size_t i = 0; i < DIM-1; ++i) {
        if (lh[i] < rh[i+1]) {
            ++counter;
            return {true, counter};
        }
        else if (lh[i] == rh[i+1]) {
            ++counter;
            continue;
        }
        else {
            counter += 2;
            return {false, counter};
        }
    }
    //Happens if both arrays coincide.
    return {true, counter};
}

inline bool lexSmallerOrEquiv(const TruncatedCosts& lh, const CostArray& rh) {
    for (size_t i = 0; i < DIM-1; ++i) {
        if (lh[i] < rh[i+1]) {
            return true;
        }
        else if (lh[i] == rh[i+1]) {
            continue;
        }
        else {
            return false;
        }
    }
    //Happens if both arrays coincide.
    return true;
}

inline bool truncatedDominance(const TruncatedFront& front, const CostArray& c) {
    if (front.empty()) {
        return false;
    }
    auto it = front.begin();
    while (it != front.end() && lexSmallerOrEquiv(*it, c)) {
        if (tc_dominates(*it, c)) {
            return true;
        }
        ++it;
    }
    return false;
}

inline const CostArray substract(const CostArray& rhs, const CostArray& lhs) {
    CostArray res;
    for (size_t i = 0; i < DIM; ++i) {
        res[i] = rhs[i] - lhs[i];
    }
    return res;
}

inline void addInPlace(CostArray& rhs, const CostArray& lhs) {
    for (size_t i = 0; i < DIM; ++i) {
        rhs[i] += lhs[i];
    }
}

inline CostArray add(const CostArray& rhs, const CostArray& lhs) {
    CostArray res;
    for (size_t i = 0; i < DIM; ++i) {
        res[i] = rhs[i] + lhs[i];
    }
    return res;
}

inline CostArray add(const CostArray& rhs, const CostArray& lhs, const DimensionsVector& dimOrdering) {
    CostArray res;
    for (size_t i = 0; i < DIM; ++i) {
        res[i] = rhs[i] + lhs[dimOrdering[i]];
    }
    return res;
}

//Dominance relationship that is also true if lhs = rhs.
inline bool dominates(const CostArray& lhs, const CostArray& rhs) {
    for (size_t i = 0; i < DIM; ++i) {
        if (lhs[i] > rhs[i]) {
            return false;
        }
    }
    return true;
    //    return lhs[0] <= rhs[0] && lhs[1] <= rhs[1] && lhs[2] <= rhs[2];
}

inline bool dominatesLexSmaller(const CostArray& lhs, const CostArray& rhs) {
    for (size_t i = 1; i < DIM; ++i) {
        if (lhs[i] > rhs[i]) {
            return false;
        }
    }
    return true;
    //    return lhs[0] <= rhs[0] && lhs[1] <= rhs[1] && lhs[2] <= rhs[2];
}

//Dominance relationship that is false if lhs = rhs.
//size_t weakDomSuccess{0};
inline bool weakDominates(const CostArray& lhs, const CostArray& rhs) {
    for (size_t i = 0; i < DIM; ++i) {
        if (lhs[i] >= rhs[i]) {
            return false;
        }
    }
    return true;
    //    return lhs[0] < rhs[0] && lhs[1] < rhs[1] && lhs[2] < rhs[2];
}

inline CostArray max(const CostArray& c1, const CostArray& c2, const DimensionsVector& dimOrdering) {
    CostArray result;
    for (size_t i = 0; i < result.size(); ++i) {
        result[dimOrdering[i]] = std::max(c1[dimOrdering[i]], c2[i]);
    }
    return result;
}

#endif
