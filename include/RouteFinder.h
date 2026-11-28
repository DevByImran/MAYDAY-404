#ifndef ROUTE_FINDER_H
#define ROUTE_FINDER_H

#include <string>
#include <iostream>
#include "SpaceGraph.h"
#include "Flight.h"

using namespace std;

struct RouteResult {
    bool reachable;
    int totalCost;
    string path[MAX_ROUTE_HOPS];
    int pathLength;

    RouteResult() : reachable(false), totalCost(-1), pathLength(0) {}
};


struct DijkstraHeapNode {
    int vertex;
    int dist;

    DijkstraHeapNode() : vertex(-1), dist(0) {}
    DijkstraHeapNode(int v, int d) : vertex(v), dist(d) {}
};

class DijkstraMinHeap {
private:
    DijkstraHeapNode heapArray[MAX_SECTORS * 4];
    int size;

    void heapifyUp(int idx);
    void heapifyDown(int idx);

public:
    DijkstraMinHeap() : size(0) {}

    void push(int v, int d);
    DijkstraHeapNode pop();
    bool isEmpty() const { return size == 0; }
    void clear() { size = 0; }
};

class RouteFinder {
private:
    const SpaceGraph* graph;

public:
    explicit RouteFinder(const SpaceGraph* g = nullptr);
    ~RouteFinder();

    void setGraph(const SpaceGraph* g) { graph = g; }

    RouteResult findFastestRoute(const string& sourceSector, const string& destinationRunway) const;
    void printRoute(const RouteResult& res) const;

   
    void runDemonstrations(SpaceGraph& mutableGraph) const;
};

#endif 
