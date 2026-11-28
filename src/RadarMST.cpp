#include "RadarMST.h"
#include <iomanip>

using namespace std;


// DisjointSet Implementation


DisjointSet::DisjointSet(int n) : count(n) {
    for (int i = 0; i < MAX_SECTORS; ++i) {
        parent[i] = i;
        rank[i] = 0;
    }
}

int DisjointSet::find(int i) {
    if (i < 0 || i >= MAX_SECTORS) return -1;
    if (parent[i] != i) {
        parent[i] = find(parent[i]); // Path compression
    }
    return parent[i];
}

bool DisjointSet::unionSets(int i, int j) {
    int rootI = find(i);
    int rootJ = find(j);

    if (rootI == rootJ) return false; // Cycle detected

    // Union by rank
    if (rank[rootI] < rank[rootJ]) {
        parent[rootI] = rootJ;
    } else if (rank[rootI] > rank[rootJ]) {
        parent[rootJ] = rootI;
    } else {
        parent[rootJ] = rootI;
        rank[rootI]++;
    }
    count--;
    return true;
}


// RadarMST Implementation


RadarMST::RadarMST(const SpaceGraph* g) : graph(g), totalEdges(0), mstEdgeCount(0), totalMSTCost(0), isGraphConnected(false) {}

RadarMST::~RadarMST() {}

int RadarMST::partition(MSTEdge arr[], int low, int high) {
    int pivot = arr[high].weight;
    int i = low - 1;

    for (int j = low; j < high; ++j) {
        if (arr[j].weight <= pivot) {
            i++;
            MSTEdge temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    MSTEdge temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return i + 1;
}

void RadarMST::customQuickSort(MSTEdge arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        customQuickSort(arr, low, pi - 1);
        customQuickSort(arr, pi + 1, high);
    }
}

void RadarMST::collectAllEdges() {
    totalEdges = 0;
    if (graph == nullptr) return;

    int n = graph->getSectorCount();
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            int w = graph->getWeight(i, j);
            if (w != GRAPH_INF && totalEdges < MAX_EDGES) {
                allEdges[totalEdges++] = MSTEdge(i, j, w, graph->getSectorName(i), graph->getSectorName(j));
            }
        }
    }
}

bool RadarMST::computeMST() {
    if (graph == nullptr) return false;
    int n = graph->getSectorCount();
    if (n <= 1) return true;

    collectAllEdges();
    customQuickSort(allEdges, 0, totalEdges - 1);

    DisjointSet ds(n);
    mstEdgeCount = 0;
    totalMSTCost = 0;

    for (int i = 0; i < totalEdges; ++i) {
        int u = allEdges[i].u;
        int v = allEdges[i].v;

        if (ds.unionSets(u, v)) {
            mstEdges[mstEdgeCount++] = allEdges[i];
            totalMSTCost += allEdges[i].weight;
            if (mstEdgeCount == n - 1) {
                break;
            }
        }
    }

    isGraphConnected = (mstEdgeCount == n - 1);
    return isGraphConnected;
}

void RadarMST::displayMST() const {
    cout << "\n================================================================================\n";
    cout << "             MODULE 9: MINIMUM RADAR COVERAGE SPANNING TREE (MST)               \n";
    cout << "================================================================================\n";
    cout << "Algorithm: Kruskal's MST (Custom Edge QuickSort & Disjoint-Set Union-Find)\n";
    cout << "Total Sectors / Vertices: " << (graph ? graph->getSectorCount() : 0) << "\n";
    cout << "MST Corridors Selected:   " << mstEdgeCount << " corridors\n";
    cout << "Graph Connectivity:       " << (isGraphConnected ? "FULLY CONNECTED" : "DISCONNECTED COMPONENTS") << "\n";
    cout << "--------------------------------------------------------------------------------\n";
    cout << left << setw(6) << "NO."
         << setw(20) << "SECTOR A"
         << setw(20) << "SECTOR B"
         << setw(10) << "WEIGHT" << "\n";
    cout << "--------------------------------------------------------------------------------\n";

    for (int i = 0; i < mstEdgeCount; ++i) {
        cout << left << setw(6) << (i + 1)
             << setw(20) << mstEdges[i].uName
             << setw(20) << mstEdges[i].vName
             << setw(10) << mstEdges[i].weight << "\n";
    }

    cout << "--------------------------------------------------------------------------------\n";
    cout << "Total Minimum Radar Coverage Weight: " << totalMSTCost << "\n";
    cout << "================================================================================\n\n";
}

bool RadarMST::verifyConnectivity() const {
    cout << "[MST Connectivity Verification]:\n";
    if (!isGraphConnected) {
        cout << "[ATC] Graph is not fully connected; cannot verify single-tree properties.\n";
        return false;
    }

    int n = graph ? graph->getSectorCount() : 0;
    bool allEdgesCritical = true;

    // Verify removing each edge individually increases component count from 1 to 2
    for (int excluded = 0; excluded < mstEdgeCount; ++excluded) {
        DisjointSet testDS(n);
        for (int i = 0; i < mstEdgeCount; ++i) {
            if (i != excluded) {
                testDS.unionSets(mstEdges[i].u, mstEdges[i].v);
            }
        }

        // Check roots
        (void)testDS.find(0);
        int componentCount = 0;
        bool counted[MAX_SECTORS] = {false};

        for (int i = 0; i < n; ++i) {
            int r = testDS.find(i);
            if (!counted[r]) {
                counted[r] = true;
                componentCount++;
            }
        }

        if (componentCount != 2) {
            allEdgesCritical = false;
        }
    }

    if (allEdgesCritical) {
        cout << " -> PASSED: Every selected MST edge is a critical bridge. Removing any edge\n"
             << "    strictly splits the radar network into 2 disconnected components.\n";
    } else {
        cout << " -> FAILED: Redundant edge detected in MST.\n";
    }

    return allEdgesCritical;
}
