//
// Created by pedro on 10.08.21.
//

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <array>
#include <cstddef> //For size_t.
#include <limits>
#include <cstdint>
#include <cinttypes>

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

//inline bool lexSmaller(const CostArray& lh, const CostArray& rh, const DimensionsVector& dimOrdering) {
//    return lh[dimOrdering[0]] < rh[dimOrdering[0]] ||
//        (lh[dimOrdering[0]] == rh[dimOrdering[0]] && lh[dimOrdering[1]] < rh[dimOrdering[1]]) ||
//        (lh[dimOrdering[0]] == rh[dimOrdering[0]] && lh[dimOrdering[1]] == rh[dimOrdering[1]] && lh[dimOrdering[2]] < rh[dimOrdering[2]]);
//}

inline bool lexSmaller(const CostArray& lh, const CostArray& rh) {
    return lh[0] < rh[0] ||
           (lh[0] == rh[0] && lh[1] < rh[1]) ||
           (lh[0] == rh[0] && lh[1] == rh[1] && lh[2] < rh[2]);
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

//Dominance relationship that is also true if lhs = rhs.
inline bool dominates(const CostArray& lhs, const CostArray& rhs) {
    return lhs[0] <= rhs[0] && lhs[1] <= rhs[1] && lhs[2] <= rhs[2];
}

//Dominance relationship that is false if lhs = rhs.
inline bool weakDominates(const CostArray& lhs, const CostArray& rhs) {
    return lhs[0] < rhs[0] && lhs[1] < rhs[1] && lhs[2] < rhs[2];
}
#endif
