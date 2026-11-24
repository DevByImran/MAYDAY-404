#ifndef RADAR_SWEEP_H
#define RADAR_SWEEP_H

#include <string>
#include <iostream>
#include "SpaceGraph.h"

using namespace std;

const int MAX_MISSING_FLIGHTS = 10;
class BFSQueue {
private:
    int data[MAX_SECTORS * 2];
    int frontIdx;
    int rearIdx;
    int count;

public:
    BFSQueue() : frontIdx(0), rearIdx(0), count(0) {}

    void enqueue(int val) {
        if (count < MAX_SECTORS * 2) {
            data[rearIdx] = val;
            rearIdx = (rearIdx + 1) % (MAX_SECTORS * 2);
            count++;
        }
    }

    int dequeue() {
        if (count == 0) return -1;
        int val = data[frontIdx];
        frontIdx = (frontIdx + 1) % (MAX_SECTORS * 2);
        count--;
        return val;
    }

    bool isEmpty() const { return count == 0; }
};

struct MissingFlight {
    string callsign;
    string airline;
    string lastKnownSector;
    int reportTick;
    int deadlineTick;
    bool isResolved;

    MissingFlight()
        : callsign(""), airline(""), lastKnownSector(""),
          reportTick(0), deadlineTick(0), isResolved(false) {}

    MissingFlight(const string& cs, const string& al, const string& sector, int repTick, int deadTick)
        : callsign(cs), airline(al), lastKnownSector(sector),
          reportTick(repTick), deadlineTick(deadTick), isResolved(false) {}
};

class RadarSweep {
private:
    const SpaceGraph* graph;

    MissingFlight missingList[MAX_MISSING_FLIGHTS];
    int missingCount;

    void dfsHelper(int u, bool visited[], int traversalOrder[], int& visitIndex) const;

public:
    explicit RadarSweep(const SpaceGraph* g = nullptr);
    ~RadarSweep();

    void setGraph(const SpaceGraph* g) { graph = g; }

    
    int bfsFrom(const string& sourceSector, bool printOutput = true) const;
    void identifyTwoHopSectors(const string& centerSector = "Control Tower") const;

    
    int dfsFrom(const string& sourceSector, bool printOutput = true) const;
    bool isReachable(const string& fromSector, const string& toSector) const;

    
    void runRunway27Demonstration() const;
    void runMissingFlightReachabilityDemo(const string& lastKnownSector) const;
// FLight missing system
    bool addMissingFlight(const string& callsign, const string& airline,
                          const string& lastKnownSector, int reportTick, int deadlineTick);
    bool searchMissingFlight(const string& callsign, int currentTick, string& resultMsg);
    bool checkExpiredMissingFlights(int currentTick, string& alertMsg);
    void displayMissingFlights() const;
    int getActiveMissingCount() const;
    void clearMissingFlights();
};

#endif 
