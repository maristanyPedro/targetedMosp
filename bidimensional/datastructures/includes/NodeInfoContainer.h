#ifndef NODECONTAINER_H
#define NODECONTAINER_H

#include <vector>
#include "NodeInfo.h" //Needed to initialize the vector member.

class Graph;

/**
 * Wraps a vector of NodeInfos. For each node in the graph, the vector has one entry. The entry containing the NodeInfo
 * for node with id n is in the vector position n.
 */
struct NodeInfoContainer {

    NodeInfoContainer(const Graph& G, bool forward);

    inline NodeInfo& getStartNode() {
        return this->getInfo(this->startNode);
    }

    inline NodeInfo& getTargetNode() {
        return this->getInfo(this->targetNode);
    }

    inline NodeInfo& getInfo(const Node n) {
        return this->nodeInfos[n];
    }

    //One NodeInfo struct per node in the graph G.
    std::vector<NodeInfo> nodeInfos;
    const Graph& G;
    const Node startNode;
    const Node targetNode;
    const bool forward;

private:
    void initialize();
};

#endif
