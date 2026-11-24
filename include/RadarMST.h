#ifndef RADAR_MST_H
#define RADAR_MST_H

#include <string>
#include <iostream>
#include "SpaceGraph.h"

using namespace std;

const int MAX_EDGES = 60;

struct MSTEdge {
    int u;
    int v;
    int weight;
    string uName;
    string vName;

    MSTEdge() : u(-1), v(-1), weight(0), uName(""), vName("") {}
    MSTEdge(int from, int to, int w, const string& un, const string& vn)
        : u(from), v(to), weight(w), uName(un), vName(vn) {}
};


class DisjointSet {
private:
    int parent[MAX_SECTORS];
    int rank[MAX_SECTORS];
    int count;

public:
    explicit DisjointSet(int n = MAX_SECTORS);

    int find(int i);
    bool unionSets(int i, int j);
    int getComponentCount() const { return count; }
};

class RadarMST {
private:
    const SpaceGraph* graph;

    MSTEdge allEdges[MAX_EDGES];
    int totalEdges;

    MSTEdge mstEdges[MAX_SECTORS];
    int mstEdgeCount;
    int totalMSTCost;
    bool isGraphConnected;

    void customQuickSort(MSTEdge arr[], int low, int high);
    int partition(MSTEdge arr[], int low, int high);

    void collectAllEdges();

public:
    explicit RadarMST(const SpaceGraph* g = nullptr);
    ~RadarMST();

    void setGraph(const SpaceGraph* g) { graph = g; }

    bool computeMST();
    void displayMST() const;
    bool verifyConnectivity() const;

    int getTotalMSTCost() const { return totalMSTCost; }
    int getMSTEdgeCount() const { return mstEdgeCount; }
};

#endif 
