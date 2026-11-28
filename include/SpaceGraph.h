#ifndef SPACE_GRAPH_H
#define SPACE_GRAPH_H

#include <string>
#include <iostream>

using namespace std;

const int MAX_SECTORS = 20;
const int GRAPH_INF = -1; // Represents no corridor

struct GraphEdgeNode {
    int destIndex;
    int weight;
    GraphEdgeNode* next;

    GraphEdgeNode(int dest, int w, GraphEdgeNode* n = nullptr)
        : destIndex(dest), weight(w), next(n) {}
};

struct SectorInfo {
    string name;
    bool isRunway;
    string description;

    SectorInfo() : name(""), isRunway(false), description("") {}
    SectorInfo(const string& n, bool runway, const string& desc = "")
        : name(n), isRunway(runway), description(desc) {}
};

class SpaceGraph {
private:
    SectorInfo sectors[MAX_SECTORS];
    int sectorCount;

    // 1. Adjacency List Representation
    GraphEdgeNode* adjList[MAX_SECTORS];

    // 2. Adjacency Matrix Representation
    int adjMatrix[MAX_SECTORS][MAX_SECTORS];

    void clearAdjList();

public:
    SpaceGraph();
    ~SpaceGraph();

    SpaceGraph(const SpaceGraph&) = delete;
    SpaceGraph& operator=(const SpaceGraph&) = delete;

    int addSector(const string& name, bool isRunway, const string& desc = "");
    int getSectorIndex(const string& name) const;
    string getSectorName(int index) const;
    bool isRunway(int index) const;
    int getSectorCount() const { return sectorCount; }

    bool addCorridor(const string& fromSector, const string& toSector, int weight);
    bool removeCorridor(const string& fromSector, const string& toSector);
    bool restoreCorridor(const string& fromSector, const string& toSector, int weight);

    
    int getWeight(int u, int v) const;
    int getWeight(const string& fromSector, const string& toSector) const;

    
    GraphEdgeNode* getAdjListHead(int u) const {
        if (u >= 0 && u < sectorCount) return adjList[u];
        return nullptr;
    }

    void displayAdjacencyList() const;
    void displayAdjacencyMatrix() const;
    void displayAirspaceSummary() const;

    void initializeDefaultAirspace();
};

#endif 
