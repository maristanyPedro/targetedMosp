//
// Created by lkraus on 18.06.21.
//

#ifndef BIDIJKSTRA_DIRECTIONMANAGER_H
#define BIDIJKSTRA_DIRECTIONMANAGER_H

//#include <functional>
#include <vector>

#include "../../datastructures/includes/typedefs.h"
#include "graph.h"

enum class SearchDirection{
    Forward,
    Backward
};

class DirectionManager {
public:

    DirectionManager(const Graph& G, Node source, Node target, SearchDirection direction):
        G{G}, relSourceNode{source}, relTargetNode{target}, direction{direction} {}

    virtual const Neighborhood & getNeighborhoodToSource(const Node& nodeId) const = 0;
    virtual const Neighborhood & getNeighborhoodToTarget(const Node& nodeId) const = 0;
    virtual bool lexSmaller(const Costs& costs, const Costs& other) const = 0;
    virtual bool dominates(const Costs& costs, const Costs& other) const = 0;
    virtual void alterHighPrioCostComponent(Costs & costs , const Costs &newCost) const = 0;
    virtual void alterSecondaryCostComponent(Costs & costs , const Costs &newCost) const = 0;

    const Graph& getGraph() const {
        return this->G;
    }

    Node getSourceId() const {
        return this->relSourceNode;
    }

    Node getTargetId() const {
        return this->relTargetNode;
    }

    SearchDirection getDirection() const {
        return this->direction;
    }

    const Graph& G;
    const Node relSourceNode;
    const Node relTargetNode;
    const SearchDirection direction;
};

class ForwardManager: public DirectionManager{

public:
    ForwardManager(const Graph& G, const Node sourceId, const Node targetId):
        DirectionManager(G, sourceId, targetId, SearchDirection::Forward) {}

    const Neighborhood& getNeighborhoodToSource(const Node& nodeId) const override {
        return backwardStar(this->G, nodeId);
    }

    const Neighborhood & getNeighborhoodToTarget(const Node& nodeId) const override {
        return forwardStar(this->G, nodeId);
    }

    inline bool lexSmaller(const Costs & costs, const Costs & other) const override {
        return costs.lexSmaller12(other);
    }

    virtual bool dominates(const Costs& costs, const Costs& other) const override {
        return costs.dominatesF(other);
    }

    inline void alterHighPrioCostComponent(Costs & costs , const Costs &newCost) const override {
        costs.c1 = newCost.c1;
    }

    inline void alterSecondaryCostComponent(Costs & costs , const Costs &newCost) const override {
        costs.c2 = newCost.c2;
    }
};

class BackwardManager: public DirectionManager{

public:
    BackwardManager(const Graph& G, const Node sourceId, const Node targetId):
        DirectionManager(G, targetId, sourceId, SearchDirection::Backward) {}

    const Neighborhood & getNeighborhoodToSource(const Node& nodeId) const override {
        return forwardStar(this->G, nodeId);
    }

    const Neighborhood & getNeighborhoodToTarget(const Node& nodeId) const override{
        return backwardStar(this->G,nodeId);
    }

    inline bool lexSmaller(const Costs & costs, const Costs & other) const override {
        return costs.lexSmaller21(other);
    }

    virtual bool dominates(const Costs& costs, const Costs& other) const override {
        return costs.dominatesB(other);
    }

    inline void alterHighPrioCostComponent(Costs & costs , const Costs &newCost) const override {
        costs.c2 = newCost.c2;
    }

    inline void alterSecondaryCostComponent(Costs & costs , const Costs &newCost) const override {
        costs.c1 = newCost.c1;
    }
};




#endif //BIDIJKSTRA_DIRECTIONMANAGER_H
