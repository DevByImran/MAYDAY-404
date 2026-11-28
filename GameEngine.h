#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <string>
#include <iostream>
#include "Flight.h"
#include "Crew.h"
#include "Registration.h"
#include "HashTable.h"
#include "EmergencyLanding.h"
#include "CommandHistory.h"
#include "BSTflight.h"
#include "AVLflight.h"
#include "SpaceGraph.h"
#include "RadarSweep.h"
#include "RouteFinder.h"
#include "RadarMST.h"
#include "LeaderBoard.h"

using namespace std;

class GameEngine {
private:
    // Game Configuration & State
    GameRules rules;
    string croName;
    int currentTick;
    int credibility;
    int flightsLanded;
    int emergenciesResolved;
    int actionsRemainingThisTurn;
    bool isGameRunning;
    bool gameWon;
    string terminationReason;

    // Active Crisis Flags
    bool activeWarIncident;
    int warSpawnTick;
    bool activeCyberAlert;
    int cyberSpawnTick;
    string cyberDisabledCorridorU;
    string cyberDisabledCorridorV;
    int cyberDisabledWeight;

    // Core Data Structures
    Registration registry;       // Primary owner of Flight objects (Singly Linked List)
    HashTable hashTable;         // O(1) Fast lookup with Linear Probing
    EmergencyLanding landingHub; // Normal Queue (FIFO) & Emergency Min Heap
    CommandHistory history;      // Doubly Linked List & Undo/Redo Stacks
    BSTflight bst;               // Binary Search Tree
    AVLflight avl;               // Self-Balancing AVL Tree
    SpaceGraph airspace;         // Adjacency List & Adjacency Matrix
    RadarSweep radar;            // BFS, DFS, and Missing Flight Registry
    RouteFinder routeFinder;     // Dijkstra's Shortest Path Algorithm
    RadarMST radarMST;           // Kruskal's Minimum Spanning Tree

    // Scenario Events
    ScenarioEvent scenarioEvents[MAX_SCENARIO_EVENTS];
    int scenarioCount;

    // Step 1: Automatic flight advance
    void advanceFlights();

    // Step 2: Automatic emergency escalation
    void escalateEmergencies();

    // Step 3: Automatic flight spawning & dynamic events
    void processScenarioEvents();

    // Step 7: Landing and cleanup helper
    void processSuccessfulLanding(Flight* f);

    // Step 8: Win / Lose checking
    bool checkWinLoseConditions();

    // Command Handlers
    bool handleClearRunway();
    bool handleRerouteFlight();
    bool handleEscalatePriority();
    bool handleInterceptHijack();
    bool handleCounterCyber();
    bool handleSearchMissing();
    bool handleUndo();
    bool handleRedo();

    // Free Information Views
    void viewManifest() const;
    void viewRadar();
    void viewAirspaceMap() const;
    void viewFastestRoute() const;
    void viewEmergencyQueues() const;
    void viewCommandHistory() const;
    void viewDataStructureStats();

    // Helper to compute planned route for flight
    void updateFlightRoute(Flight* f);

public:
    GameEngine();
    ~GameEngine();

    bool initializeSimulation();
    void startNewGame(const string& playerName = "");
    void runTurn();
    void executeGameLoop();
    void displayDashboard() const;
    void displayGameOverScreen();

    // Fast-turn execution for unit testing without interactive cin
    bool executeAction(int actionChoice, const string& param1 = "", const string& param2 = "");
    void advanceOneTick();

    // Getters for game state inspection
    int getCurrentTick() const { return currentTick; }
    int getCredibility() const { return credibility; }
    int getFlightsLanded() const { return flightsLanded; }
    int getEmergenciesResolved() const { return emergenciesResolved; }
    bool isRunning() const { return isGameRunning; }
    bool isVictory() const { return gameWon; }
    string getTerminationReason() const { return terminationReason; }
};

#endif // GAME_ENGINE_H
