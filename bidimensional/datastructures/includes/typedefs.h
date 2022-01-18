//
// Created by pedro on 10.08.21.
//

#ifndef BIDIJKSTRA_TYPEDEFS_H
#define BIDIJKSTRA_TYPEDEFS_H

#include <array>
#include <cstddef> //For size_t.
#include <limits>
#include <stdint.h>
#include <cinttypes>

typedef uint32_t Node;
typedef uint16_t NeighborhoodSize;
typedef uint32_t ArcId;
typedef uint32_t CostType;

constexpr Node INVALID_NODE = std::numeric_limits<Node>::max();
constexpr NeighborhoodSize MAX_DEGREE = std::numeric_limits<NeighborhoodSize>::max();
constexpr ArcId INVALID_ARC = std::numeric_limits<ArcId>::max();
constexpr CostType MAX_COST = std::numeric_limits<CostType>::max();
constexpr uint16_t MAX_PATH = std::numeric_limits<uint16_t>::max();

template <typename T>
using Info = std::array<T, 2>;
typedef Info<CostType> CostArray;

#endif //BIDIJKSTRA_TYPEDEFS_H
