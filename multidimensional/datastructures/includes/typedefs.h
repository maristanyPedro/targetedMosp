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

constexpr Dimension DIM = 3;

constexpr Node INVALID_NODE = std::numeric_limits<Node>::max();
constexpr NeighborhoodSize MAX_DEGREE = std::numeric_limits<NeighborhoodSize>::max();
constexpr ArcId INVALID_ARC = std::numeric_limits<ArcId>::max();
constexpr CostType MAX_COST = std::numeric_limits<CostType>::max();
constexpr uint16_t MAX_PATH = std::numeric_limits<uint16_t>::max();

template <typename T>
using Info = std::array<T, DIM>;
typedef Info<CostType> CostArray;
typedef Info<Dimension> DimensionsVector;

template <typename T>
using DimReductedInfo = std::array<T, DIM-1>;
typedef DimReductedInfo<CostType> TruncatedCosts;
typedef std::list<TruncatedCosts> TruncatedFront;

inline bool lexSmaller(const CostArray& lh, const CostArray& rh) {
    return lh[0] < rh[0] ||
           (lh[0] == rh[0] && lh[1] < rh[1]) ||
           (lh[0] == rh[0] && lh[1] == rh[1] && lh[2] < rh[2]);
}

inline CostArray substract(const CostArray& rhs, const CostArray& lhs) {
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

//Dominance relationship that is also true if lhs = rhs.
inline bool dominates(const CostArray& lhs, const CostArray& rhs) {
    return lhs[0] <= rhs[0] && lhs[1] <= rhs[1] && lhs[2] <= rhs[2];
}

//Dominance relationship that is false if lhs = rhs.
inline bool weakDominates(const CostArray& lhs, const CostArray& rhs) {
    return lhs[0] < rhs[0] && lhs[1] < rhs[1] && lhs[2] < rhs[2];
}

inline CostArray max(const CostArray& c1, const CostArray& c2, const DimensionsVector& dimOrdering) {
    CostArray result;
    for (size_t i = 0; i < result.size(); ++i) {
        result[dimOrdering[i]] = std::max(c1[dimOrdering[i]], c2[i]);
    }
    return result;
}

#endif
