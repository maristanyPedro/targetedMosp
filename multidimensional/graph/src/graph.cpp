#include <cassert>
#include <fstream>
#include <iterator>
#include <sstream>
#include <iostream>
#include <string>
#include <random>
#include <filesystem>

#include "../includes/graph.h"

using namespace std;

void Graph::exportGraph() const {
    stringstream filenamess;
    filenamess << std::to_string(DIM) << "d_" << this->name;
    string filename = filenamess.str();
    string outputDirectory = "../instances/";
    string outputFile = outputDirectory.append(filename);
//    if (std::filesystem::exists(outputFile)) {
//        printf("File %s, a %u dimensional version of your input file already exists. Please call the program using this file as input.\n",
//               outputFile.c_str(), DIM);
//        exit(1);
//    }
    printf("Your instance file does not match the compile dimension DIM. I will generate the file %s and abort this run. Use the new file as input.\n", outputFile.c_str());
    ofstream outputfile(outputFile);
    string headerLine = "p sp " + std::to_string(this->nodesCount) + " " + std::to_string(this->arcsCount) + "\n";
    outputfile << headerLine;
    for (Node n = 0; n < this->nodesCount; ++n) {
        for (const Arc& arc : this->outgoingArcs(n)) {
            string arcLine = "a " + std::to_string(n) + " " + std::to_string(arc.n);
            for (Dimension d = 0; d < DIM; ++d) {
                arcLine.append(" ");
                arcLine.append(std::to_string(arc.c[d]));
            }
            arcLine.append("\n");
            outputfile << arcLine;
        }
    }
    outputfile.close();
}

unique_ptr<Graph> setupGraph(const string& filename, Node sourceId, Node targetId) {
    assert (sourceId != INVALID_NODE && targetId != INVALID_NODE);
    ifstream infile(filename);
    string line;
    size_t nodesCount =0, arcsCount = 0;
#ifdef GENERATE_MISSING_COST_COMPONENTS_UNIF_RANDOM
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(1, 10); // define the range
#endif
    //Parse file until information about number of nodes and number of arcs is reached.
    while (getline(infile, line)) {
        vector<string> splittedLine{split(line, ' ')};
        if (splittedLine[0] == "p") {
            nodesCount = stoi(splittedLine[2]);
            arcsCount = stoi(splittedLine[3]);
            break;
        }
    }
    if (nodesCount == 0 || arcsCount == 0) {
        printf("Could not determine the size of the graph %s. Abort.\n", filename.c_str());
        exit(1);
    }
    std::vector<NeighborhoodSize> inDegree(nodesCount, 0);
    std::vector<NeighborhoodSize> outDegree(nodesCount, 0);
    size_t addedArcs = 0;
    std::vector<std::pair<Node,Arc>> arcs(arcsCount);
    bool costComponentsGenerated = false;
    while (getline(infile, line)) {
        vector<string> splittedLine{split(line, ' ')};
        if (splittedLine[0] == "a") {
            //If not, you are dealing with a higher dimensional problem!
            //assert(splittedLine.size() == 6);
            Node tailId;
            std::stringstream(splittedLine[1]) >> tailId;
            Node headId;
            std::stringstream(splittedLine[2]) >> headId;
            ++outDegree[tailId];
            assert(outDegree[tailId] != MAX_DEGREE);
            ++inDegree[headId];
            assert(inDegree[headId] != MAX_DEGREE);
            CostArray arcCost = generate();
            size_t costComponent;
            for (costComponent = 0; costComponent + 3 < splittedLine.size(); ++costComponent) {
                arcCost[costComponent] = stoi(splittedLine[3 + costComponent]);
            }
            if (costComponent < DIM) {
                costComponentsGenerated = true;
            }
            while (costComponent < DIM) {
                assert(arcCost[costComponent] == MAX_COST);
#ifdef GENERATE_MISSING_COST_COMPONENTS_UNIF_RANDOM
                arcCost[costComponent] = distr(gen);
#else
                arcCost[costComponent] = 1;
#endif
                ++costComponent;
            }
            arcs[addedArcs] = make_pair(tailId, Arc(headId, arcCost, addedArcs));
            ++addedArcs;
        }
    }
    assert(addedArcs == arcsCount);
    unique_ptr<Graph> G = make_unique<Graph>(nodesCount, arcsCount, sourceId, targetId);
    G->costComponentAdded = costComponentsGenerated;
    for (Node i = 0; i < nodesCount; ++i) {
        G->setNodeInfo(i, inDegree[i], outDegree[i]);
    }
    vector<NeighborhoodSize> incomingArcsPerNode(nodesCount, 0);
    vector<NeighborhoodSize> outgoingArcsPerNode(nodesCount, 0);
    for (auto& arc: arcs) {
        NodeAdjacency& tail = G->node(arc.first);
        NodeAdjacency& head = G->node(arc.second.n);
        tail.id = arc.first;
        head.id = arc.second.n;
        //The arc currently represents an arc as an outgoing arc from the arc's tail.s
        arc.second.revArcIndex = incomingArcsPerNode[head.id];
        tail.outgoingArcs[outgoingArcsPerNode[tail.id]++] = arc.second;
        //Update arc-entries s.t. the updated object represents the same arc but as an incoming arc to the arc's head.
        arc.second.n = tail.id;
        arc.second.revArcIndex = outgoingArcsPerNode[tail.id] - 1;
        head.incomingArcs[incomingArcsPerNode[head.id]++] = arc.second;

    }
    for (Node i = 0; i < nodesCount; ++i) {
        assert(inDegree[i] == incomingArcsPerNode[i]);
        assert(outDegree[i] == outgoingArcsPerNode[i]);
    }
    return G;
}

void Graph::setNodeInfo(Node n, NeighborhoodSize inDegree, NeighborhoodSize outDegree) {
    NodeAdjacency& currentNode = this->nodes[n];
    currentNode.id = n;
    currentNode.incomingArcs.resize(inDegree);
    currentNode.outgoingArcs.resize(outDegree);
}

void Graph::printNodeInfo(const Node nodeId) const {
    printf("Analyzing node: %u\n", nodeId);
    printf("OUTGOING ARCS\n");
    printArcs(this->outgoingArcs(nodeId));
    printf("INCOMING ARCS\n");
    printArcs(this->incomingArcs(nodeId));
}

void Graph::printArcs(const Neighborhood & arcs) {
    for (const Arc& arc : arcs) {
        arc.print();
    }
}

Arc::Arc(const Node targetId, const CostArray& cArray, const ArcId id):
        n{targetId}, c{cArray}, id{id} {}


void Arc::print() const {
    printf("Arc costs: (%u, %u, %u)\n", c[0], c[1], c[2]);
}

Graph::Graph(Node nodesCount, ArcId arcsCount, Node source, Node target):
    nodesCount{nodesCount},
    arcsCount{arcsCount},
    source{source},
    target{target},
    nodes(nodesCount) {
    //We need that many entries in our ncl-vectors in MDA.
    //In case this assertion fails, just change the ArcId typedef in typedefs.h
    assert(INVALID_ARC >= 2*arcsCount);
}

void split(const string& s, char delim, vector<string>& elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

vector<string> split(const string& s, char delim) {
    vector<string> elems;
    if (delim == ' ') {
        istringstream iss(s);
        elems = {istream_iterator<string>{iss}, istream_iterator<string>{}};
    } else {
        split(s, delim, elems);
    }

    for (auto& elem : elems) {
        if (elem.back() == '\n') {
            elem = elem.substr(0, elem.size() - 1);
        }
    }
    return elems;
}

NodeAdjacency::NodeAdjacency(): id{INVALID_NODE} {}

NodeAdjacency::NodeAdjacency(const Node nid, size_t incomingArcsCt, size_t outgoingArcsCt):
    incomingArcs(incomingArcsCt),
    outgoingArcs(outgoingArcsCt),
    id{nid} {

    incomingArcs.shrink_to_fit();
    outgoingArcs.shrink_to_fit();
}
