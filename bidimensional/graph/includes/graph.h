#ifndef GRAPH_H_
#define GRAPH_H_

#include <memory> //for unique_ptr
#include <set>
#include <string>
#include <list>
#include <vector>

#include "../../datastructures/includes/typedefs.h"

struct Arc {
    Arc() = default;
    Arc(const Node targetId, const CostType c1, const CostType c2);

    void operator=(const Arc& other) {
        this->n = other.n; this->c = other.c; this->revArcIndex = other.revArcIndex;
    }

    void print() const;

    Node n{INVALID_NODE};

    Info<CostType> c{MAX_COST, MAX_COST};
    uint16_t lastProcessedPred{0};

    NeighborhoodSize revArcIndex{MAX_DEGREE};
};

typedef std::vector<Arc> Neighborhood;

class NodeAdjacency {
    public:
        NodeAdjacency();
        NodeAdjacency(const Node nid, std::size_t incomingArcsCt, std::size_t outgoingArcsCt);
    //private:
        Neighborhood incomingArcs;
        Neighborhood outgoingArcs;
        Node id;
};

class Graph {
    public:
        Graph(Node nodesCount, ArcId arcsCount, Node source, Node target);

        inline const Neighborhood& outgoingArcs(const Node nodeId) const {
            return this->nodes[nodeId].outgoingArcs;
        }

        inline const Neighborhood& incomingArcs(const Node nodeId) const {
            return this->nodes[nodeId].incomingArcs;
        }

        inline Neighborhood& outgoingArcs(const Node nodeId) {
            return this->nodes[nodeId].outgoingArcs;
        }

        inline Neighborhood& incomingArcs(const Node nodeId) {
            return this->nodes[nodeId].incomingArcs;
        }

        const NodeAdjacency& node(const Node nodeId) const;
        NodeAdjacency& node(const Node nodeId);

        void printNodeInfo(const Node nodeId) const;
        void printArcs(const Neighborhood & arcs) const;

        void setNodeInfo(Node n, NeighborhoodSize inDegree, NeighborhoodSize outDegree);

    const Node nodesCount;
    const ArcId arcsCount;
    const Node source;
    const Node target;

    std::string name;

    private: //Members
        std::vector<NodeAdjacency> nodes;
};

inline const NodeAdjacency& Graph::node(const Node nodeId) const {
    return this->nodes[nodeId];
}

inline NodeAdjacency& Graph::node(const Node nodeId) {
    return this->nodes[nodeId];
}

std::unique_ptr<Graph> setupGraph(const std::string& filename, Node sourceId, Node targetId);

inline const Neighborhood& forwardStar(const Graph& G, const Node nodeId) {
    return G.outgoingArcs(nodeId);
}
inline const Neighborhood & backwardStar(const Graph& G, const Node nodeId) {
    return G.incomingArcs(nodeId);
}

void split(const std::string& s, char delim, std::vector<std::string>& elems);

std::vector<std::string> split(const std::string& s, char delim);

#endif
