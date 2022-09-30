#ifndef SOLUTIONS_LIST_H_
#define SOLUTIONS_LIST_H_

#include <list>
#include "../../datastructures/includes/typedefs.h"

struct Label;
class Graph;
class Permanents;

class SolutionsList {

public:
    std::list<Label*> solutions;

    void printSolutions(const Graph& G, const Permanents& permanents) const;
};


#endif