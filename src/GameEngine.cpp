#include "GameEngine.h"
#include <iomanip>
#include <ctime>
#include <sstream>

using namespace std;

GameEngine::GameEngine()
    : currentTick(1), credibility(100), flightsLanded(0), emergenciesResolved(0),
      actionsRemainingThisTurn(2), isGameRunning(false), gameWon(false),
      terminationReason(""), activeWarIncident(false), warSpawnTick(0),
      activeCyberAlert(false), cyberSpawnTick(0), cyberDisabledCorridorU(""),
      cyberDisabledCorridorV(""), cyberDisabledWeight(0), scenarioCount(0) {
    radar.setGraph(&airspace);
    routeFinder.setGraph(&airspace);
    radarMST.setGraph(&airspace);
}

GameEngine::~GameEngine() {}

bool GameEngine::initializeSimulation() {
    LeaderBoard::loadRules("data/rules.txt", rules);
    credibility = rules.startingCredibility;

    LeaderBoard::loadAirspace("data/airspace.txt", airspace);
    radar.setGraph(&airspace);
    routeFinder.setGraph(&airspace);
    radarMST.setGraph(&airspace);

    scenarioCount = LeaderBoard::loadScenario("data/scenario.txt", scenarioEvents, MAX_SCENARIO_EVENTS);
    LeaderBoard::loadCrew("data/crew.txt", registry);
    LeaderBoard::loadFlights("data/flights.txt", registry, hashTable, bst, avl, landingHub);

    return true;
}

void GameEngine::startNewGame(const string& playerName) {
    string name = playerName;
    if (name.empty()) {
        while (true) {
            cout << "\nEnter Chief Radar Officer (CRO) Callsign: ";
            string input;
            if (!getline(cin, input)) {
                name = "OFFICER_DEFAULT";
                break;
            }
            input = LeaderBoard::trim(input);
            if (input.empty()) {
                cout << "[ATC Error] CRO Name cannot be empty.\n";
                continue;
            }
            if (input.find(' ') != string::npos) {
                cout << "[ATC Error] Name cannot contain spaces. Use '_' or '.' instead (e.g. CRO_TALHA).\n";
                continue;
            }
            name = input;
            break;
        }
    }
    croName = name;

    // Reset game state
    currentTick = 1;
    credibility = rules.startingCredibility;
    flightsLanded = 0;
    emergenciesResolved = 0;
    isGameRunning = true;
    gameWon = false;
    terminationReason = "";

    activeWarIncident = false;
    warSpawnTick = 0;
    activeCyberAlert = false;
    cyberSpawnTick = 0;

    // Reset data structures
    registry.destroyList();
    hashTable.clear();
    bst.destroyTree();
    avl.destroyTree();
    landingHub.removeFlightFromAll("");
    history.clearAll();
    radar.clearMissingFlights();

    // Reload initial data
    initializeSimulation();
    actionsRemainingThisTurn = rules.actionsPerTurn;

    // Compute initial routes for flights
    FlightNode* curr = registry.getHead();
    while (curr != nullptr) {
        if (curr->flight != nullptr) {
            updateFlightRoute(curr->flight);
        }
        curr = curr->next;
    }

    history.logCommand("SIMULATION_STARTED", "SYSTEM", currentTick);
    cout << "\n[ATC Radar System Online] Welcome, Chief Radar Officer " << croName << "!\n";
}

void GameEngine::updateFlightRoute(Flight* f) {
    if (f == nullptr) return;
    RouteResult res = routeFinder.findFastestRoute(f->currentSector, f->assignedRunway);
    if (res.reachable) {
        f->setRoute(res.path, res.pathLength);
    }
}

void GameEngine::advanceFlights() {
    FlightNode* curr = registry.getHead();
    while (curr != nullptr && isGameRunning) {
        Flight* f = curr->flight;
        FlightNode* nextNode = curr->next; // Save next before potential deletion

        if (f != nullptr && f->status == "ACTIVE") {
            // Decrement remaining fuel
            f->fuelRemaining--;

            // Fuel exhaustion check
            if (f->fuelRemaining <= 0) {
                f->status = "CRASHED";
                credibility -= rules.penaltyCrash;
                history.logCommand("CRASH_FUEL_DEPLETED", f->callsign, currentTick);
                terminationReason = "Flight " + f->callsign + " crashed due to complete fuel starvation!";
                isGameRunning = false;
                return;
            }

            // Check if ready and cleared to land
            bool atOrAdjacentToRunway = (f->currentSector == f->assignedRunway ||
                                         airspace.getWeight(f->currentSector, f->assignedRunway) != GRAPH_INF);
            if (atOrAdjacentToRunway && f->clearedForLanding) {
                processSuccessfulLanding(f);
                curr = nextNode;
                continue;
            }

            // Route advance towards runway
            if (f->routeLength == 0 || f->plannedRoute[f->routeStep] != f->currentSector) {
                updateFlightRoute(f);
            }

            if (f->routeStep + 1 < f->routeLength) {
                string nextSector = f->plannedRoute[f->routeStep + 1];
                bool nextIsRunway = airspace.isRunway(airspace.getSectorIndex(nextSector));
                if (nextIsRunway) {
                    if (f->clearedForLanding) {
                        f->currentSector = nextSector;
                        f->routeStep++;
                        processSuccessfulLanding(f);
                        curr = nextNode;
                        continue;
                    } else {
                        // Aircraft holds in current sector awaiting landing clearance
                    }
                } else if (airspace.getWeight(f->currentSector, nextSector) != GRAPH_INF) {
                    f->currentSector = nextSector;
                    f->routeStep++;
                } else {
                    // Corridor blocked by cyber attack - recalculate route
                    updateFlightRoute(f);
                }
            }
        }
        curr = nextNode;
    }
}

void GameEngine::escalateEmergencies() {
    // 1. War incident escalation
    if (activeWarIncident) {
        if (currentTick - warSpawnTick >= rules.deadlineWar) {
            isGameRunning = false;
            gameWon = false;
            terminationReason = "Unidentified hostile combat aircraft breached national airspace perimeter!";
            return;
        }
    }

    // 2. Cyber alert escalation
    if (activeCyberAlert) {
        int cyberAge = currentTick - cyberSpawnTick;
        if (cyberAge == rules.deadlineCyber && cyberDisabledCorridorU.empty()) {
            // Disable one corridor
            cyberDisabledCorridorU = "Syntax Sector";
            cyberDisabledCorridorV = "Runway 12";
            cyberDisabledWeight = airspace.getWeight(cyberDisabledCorridorU, cyberDisabledCorridorV);
            airspace.removeCorridor(cyberDisabledCorridorU, cyberDisabledCorridorV);
            credibility -= rules.penaltyCyberDisable;
            cout << "\n[CRITICAL CYBER BREACH] Attackers jammed navigation corridor between "
                 << cyberDisabledCorridorU << " and " << cyberDisabledCorridorV << "!\n";
        } else if (cyberAge > rules.deadlineCyber) {
            isGameRunning = false;
            gameWon = false;
            terminationReason = "Cyber attack compromised central tower mainframe! Total ATC blackout!";
            return;
        }
    }

    // 3. Missing flight check
    string missingAlert;
    if (radar.checkExpiredMissingFlights(currentTick, missingAlert)) {
        cout << "\n" << missingAlert << "\n";
        credibility -= rules.penaltyBreach;
        history.logCommand("RESTRICTED_ZONE_BREACH", "MISSING_FLT", currentTick);
        if (credibility <= 0) {
            isGameRunning = false;
            gameWon = false;
            terminationReason = "Security breach: Unidentified missing flight penetrated Restricted Zone!";
            return;
        }
    }

    // 4. Flight specific emergency escalation
    FlightNode* curr = registry.getHead();
    while (curr != nullptr) {
        Flight* f = curr->flight;
        if (f != nullptr && f->status == "ACTIVE" && !f->isEmergencyResolved) {
            int age = currentTick - f->arrivalTick;

            if (f->priorityLevel == 2 && age >= rules.deadlineHijack) { // Hijack
                isGameRunning = false;
                gameWon = false;
                terminationReason = "Hijackers seized control of flight " + f->callsign + "!";
                return;
            } else if (f->priorityLevel == 6 && age >= rules.deadlineMedical) { // Medical
                credibility -= rules.penaltyMedical;
                f->isEmergencyResolved = true;
                cout << "\n[ATC Incident] Medical deadline expired on " << f->callsign
                     << "! Critical passenger casualty incurred. Credibility -" << rules.penaltyMedical << ".\n";
            } else if (f->priorityLevel == 7 && age >= rules.deadlineWeather) { // Weather
                isGameRunning = false;
                gameWon = false;
                terminationReason = "Flight " + f->callsign + " encountered violent storm windshear and crashed!";
                return;
            } else if (f->priorityLevel == 8 && age >= rules.deadlineMechanical) { // Mechanical
                isGameRunning = false;
                gameWon = false;
                terminationReason = "Flight " + f->callsign + " suffered structural flight control failure and crashed!";
                return;
            }
        }
        curr = curr->next;
    }
}

void GameEngine::processScenarioEvents() {
    for (int i = 0; i < scenarioCount; ++i) {
        if (scenarioEvents[i].tick == currentTick && !scenarioEvents[i].triggered) {
            scenarioEvents[i].triggered = true;

            if (scenarioEvents[i].type == EVENT_SPAWN) {
                Flight sf = scenarioEvents[i].spawnFlight;
                if (hashTable.find(sf.callsign) == nullptr) {
                    Flight* newFlight = new Flight(sf.callsign, sf.aircraftType, sf.airline,
                                                   sf.origin, sf.destination, sf.passengerCount,
                                                   sf.fuelRemaining, sf.currentSector,
                                                   sf.assignedRunway, currentTick,
                                                   sf.priorityLevel, sf.status);
                    updateFlightRoute(newFlight);
                    registry.addFlight(newFlight);
                    hashTable.insert(sf.callsign, newFlight);
                    bst.insert(newFlight);
                    avl.insert(newFlight);

                    if (newFlight->priorityLevel == 9) {
                        landingHub.enqueueNormal(newFlight);
                    } else {
                        landingHub.insertEmergency(newFlight);
                    }

                    cout << "\n[ATC RADAR CONTACT] New inbound flight " << sf.callsign
                         << " (" << sf.aircraftType << ") entered sector " << sf.currentSector
                         << " [Priority: " << Flight::getPriorityName(sf.priorityLevel) << "]!\n";
                }
            } else if (scenarioEvents[i].type == EVENT_CYBER) {
                activeCyberAlert = true;
                cyberSpawnTick = currentTick;
                cout << "\n[CYBER THREAT ALERT] " << scenarioEvents[i].description << "\n";
            } else if (scenarioEvents[i].type == EVENT_MISSING) {
                radar.addMissingFlight(scenarioEvents[i].target, "EgyptAir", "Alpha Tree", currentTick, currentTick + rules.deadlineMissing);
                cout << "\n[TRANSPONDER LOST] " << scenarioEvents[i].description << "\n";
            } else if (scenarioEvents[i].type == EVENT_WAR) {
                activeWarIncident = true;
                warSpawnTick = currentTick;
                cout << "\n[MILITARY DEFENSE ALERT] " << scenarioEvents[i].description << "\n";
            } else if (scenarioEvents[i].type == EVENT_WEATHER) {
                cout << "\n[SEVERE WEATHER ALERT] " << scenarioEvents[i].description << "\n";
            } else if (scenarioEvents[i].type == EVENT_MECHANICAL) {
                Flight* f = hashTable.find(scenarioEvents[i].target);
                if (f != nullptr) {
                    f->priorityLevel = 8;
                    landingHub.insertEmergency(f);
                }
                cout << "\n[MECHANICAL EMERGENCY] " << scenarioEvents[i].description << "\n";
            }
        }
    }
}

void GameEngine::processSuccessfulLanding(Flight* f) {
    if (f == nullptr) return;
    string cs = f->callsign;

    // Remove from indexes
    hashTable.remove(cs);
    bst.erase(cs);
    avl.erase(cs);
    landingHub.removeFlightFromAll(cs);

    // Remove from primary owner registry (this frees flight memory safely)
    registry.removeFlight(cs);

    flightsLanded++;
    credibility += rules.rewardLanding;
    history.logCommand("LANDED_SAFELY", cs, currentTick);

    cout << "\n[ATC SUCCESS] Flight " << cs << " landed safely! Airport Credibility +"
         << rules.rewardLanding << " (Now: " << credibility << ")\n";
}

bool GameEngine::checkWinLoseConditions() {
    if (credibility <= 0) {
        isGameRunning = false;
        gameWon = false;
        terminationReason = "Airport credibility plummeted to zero! Airport shutdown!";
        return true;
    }
    if (currentTick >= rules.winTick) {
        isGameRunning = false;
        gameWon = true;
        terminationReason = "Chief Radar Officer successfully secured airport operations across all 30 ticks!";
        return true;
    }
    return !isGameRunning;
}

void GameEngine::displayDashboard() const {
    cout << "\n================================================================================\n";
    cout << "                          ATC RADAR OPERATIONS CENTER                          \n";
    cout << "================================================================================\n";
    cout << " CRO: " << left << setw(18) << croName
         << " TICK: " << setfill('0') << setw(2) << currentTick << " / " << rules.winTick << setfill(' ')
         << "      CREDIBILITY: " << credibility
         << "      LANDED: " << flightsLanded << "\n";
    cout << "--------------------------------------------------------------------------------\n";

    if (activeWarIncident) {
        cout << " [!] ACTIVE WAR ALERT: Hostile combat intrusion! Intercept deadline in "
             << (rules.deadlineWar - (currentTick - warSpawnTick)) << " tick(s)!\n";
    }
    if (activeCyberAlert) {
        cout << " [!] ACTIVE CYBER ALERT: Network intrusion! Jamming threat active!\n";
    }
    if (radar.getActiveMissingCount() > 0) {
        cout << " [!] MISSING AIRCRAFT: Search operation pending in radar archives!\n";
    }

    cout << "\nACTIVE AIRSPACE FLIGHTS:\n";
    cout << left << setw(9) << "CALLSIGN"
         << setw(11) << "TYPE"
         << setw(16) << "SECTOR"
         << setw(12) << "RUNWAY"
         << setw(6) << "FUEL"
         << setw(12) << "PRIORITY"
         << setw(10) << "STATUS" << "\n";
    cout << "--------------------------------------------------------------------------------\n";

    FlightNode* curr = registry.getHead();
    int count = 0;
    while (curr != nullptr) {
        if (curr->flight != nullptr) {
            curr->flight->displaySummary();
            count++;
        }
        curr = curr->next;
    }
    if (count == 0) {
        cout << "Airspace is currently clear.\n";
    }

    cout << "--------------------------------------------------------------------------------\n";
    cout << "ACTIONS REMAINING THIS TURN: " << actionsRemainingThisTurn << " / " << rules.actionsPerTurn << "\n";
    cout << "================================================================================\n";
    cout << "1. Clear Flight for Runway   2. Reroute Flight            3. Escalate Priority\n";
    cout << "4. Intercept Hijack          5. Counter Cyber Alert       6. Search Missing Flight\n";
    cout << "7. Undo Last Command         8. Redo Command              9. View Flight Manifest [Free]\n";
    cout << "10. View Radar (BFS/DFS)[Free] 11. View Airspace Map [Free] 12. View Fastest Route [Free]\n";
    cout << "13. View Queues/Heap [Free]  14. View Command History [Free] 15. View Data Structures [Free]\n";
    cout << "16. End Turn / Advance Tick  17. Abort Simulation\n";
    cout << "--------------------------------------------------------------------------------\n";
    cout << "Enter Action Choice (1 - 17): ";
}

bool GameEngine::handleClearRunway() {
    cout << "\nEnter Flight Callsign to clear for landing: ";
    string cs;
    cin >> cs;
    Flight* f = hashTable.find(cs);
    if (f == nullptr) {
        cout << "[ATC Error] Flight '" << cs << "' not found in active airspace.\n";
        return false;
    }

    // Save snapshot for Undo
    FlightSnapshot snap(f->callsign, f->assignedRunway, f->priorityLevel,
                       f->currentSector, f->status, f->isEmergencyResolved,
                       currentTick, "CLEAR_RUNWAY");
    history.pushUndoSnapshot(snap);
    history.clearRedoStack();

    f->clearedForLanding = true;
    history.logCommand("CLEAR_RUNWAY", cs, currentTick);
    cout << "[ATC] Flight " << cs << " cleared for landing on " << f->assignedRunway << ".\n";
    return true;
}

bool GameEngine::handleRerouteFlight() {
    cout << "\nEnter Flight Callsign to reroute: ";
    string cs;
    cin >> cs;
    Flight* f = hashTable.find(cs);
    if (f == nullptr) {
        cout << "[ATC Error] Flight '" << cs << "' not found.\n";
        return false;
    }

    cout << "Available Runways: Runway 12 (West), Runway 04 (East), Runway 17 (South), Runway 09 (North)\n";
    cout << "Enter New Assigned Runway: ";
    string newRunway;
    cin.ignore();
    getline(cin, newRunway);
    newRunway = LeaderBoard::trim(newRunway);

    if (airspace.getSectorIndex(newRunway) == -1 || !airspace.isRunway(airspace.getSectorIndex(newRunway))) {
        cout << "[ATC Error] Invalid runway assignment: '" << newRunway << "'.\n";
        return false;
    }

    FlightSnapshot snap(f->callsign, f->assignedRunway, f->priorityLevel,
                       f->currentSector, f->status, f->isEmergencyResolved,
                       currentTick, "REROUTE");
    history.pushUndoSnapshot(snap);
    history.clearRedoStack();

    f->assignedRunway = newRunway;
    updateFlightRoute(f);
    if (f->priorityLevel == 7) { // Weather emergency resolved by reroute!
        f->isEmergencyResolved = true;
        emergenciesResolved++;
        cout << "[ATC] Severe weather hazard avoided! Emergency marked resolved.\n";
    }

    history.logCommand("REROUTE", cs, currentTick);
    cout << "[ATC] Flight " << cs << " successfully rerouted to " << newRunway << ".\n";
    return true;
}

bool GameEngine::handleEscalatePriority() {
    cout << "\nEnter Flight Callsign to escalate: ";
    string cs;
    cin >> cs;
    Flight* f = hashTable.find(cs);
    if (f == nullptr) {
        cout << "[ATC Error] Flight not found.\n";
        return false;
    }

    cout << "Enter New Priority Level (1=War, 2=Hijack, 5=Fuel, 6=Medical, 7=Weather, 8=Mechanical): ";
    int p;
    if (!(cin >> p) || p < 1 || p > 9) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "[ATC Error] Invalid priority level.\n";
        return false;
    }

    FlightSnapshot snap(f->callsign, f->assignedRunway, f->priorityLevel,
                       f->currentSector, f->status, f->isEmergencyResolved,
                       currentTick, "ESCALATE_PRIORITY");
    history.pushUndoSnapshot(snap);
    history.clearRedoStack();

    landingHub.removeFlightFromAll(f->callsign);
    f->priorityLevel = p;
    landingHub.insertEmergency(f);

    history.logCommand("ESCALATE_PRIORITY", cs, currentTick);
    cout << "[ATC] Priority of " << cs << " escalated to " << Flight::getPriorityName(p) << ".\n";
    return true;
}

bool GameEngine::handleInterceptHijack() {
    if (activeWarIncident) {
        cout << "\n[AIR DEFENSE SCRAMBLE] Scrambling fighter interceptors against military intrusion!\n";
        FlightSnapshot snap("MILITARY_INTRUSION", "", 1, "", "", activeWarIncident, currentTick, "INTERCEPT_WAR");
        history.pushUndoSnapshot(snap);
        history.clearRedoStack();

        activeWarIncident = false;
        emergenciesResolved++;
        history.logCommand("INTERCEPT_WAR", "PAF_AIR_DEFENSE", currentTick);
        cout << "[ATC] Hostile target intercepted and escorted out of national airspace!\n";
        return true;
    }

    cout << "\nEnter Callsign of hijacked flight to intercept: ";
    string cs;
    cin >> cs;
    Flight* f = hashTable.find(cs);
    if (f == nullptr || f->priorityLevel != 2) {
        cout << "[ATC Error] Callsign is not currently classified as a Hijack emergency.\n";
        return false;
    }

    FlightSnapshot snap(f->callsign, f->assignedRunway, f->priorityLevel,
                       f->currentSector, f->status, f->isEmergencyResolved,
                       currentTick, "INTERCEPT_HIJACK");
    history.pushUndoSnapshot(snap);
    history.clearRedoStack();

    f->isEmergencyResolved = true;
    f->priorityLevel = 9; // Neutralized
    landingHub.removeFlightFromAll(f->callsign);
    landingHub.enqueueNormal(f);
    emergenciesResolved++;

    history.logCommand("INTERCEPT_HIJACK", cs, currentTick);
    cout << "[ATC] Tactical security forces intercepted and secured flight " << cs << "!\n";
    return true;
}

bool GameEngine::handleCounterCyber() {
    if (!activeCyberAlert) {
        cout << "[ATC Error] No active cyber attack in progress.\n";
        return false;
    }

    FlightSnapshot snap("CYBER_SYSTEM", "", 3, "", "", activeCyberAlert, currentTick, "COUNTER_CYBER");
    history.pushUndoSnapshot(snap);
    history.clearRedoStack();

    activeCyberAlert = false;
    if (!cyberDisabledCorridorU.empty()) {
        airspace.restoreCorridor(cyberDisabledCorridorU, cyberDisabledCorridorV, cyberDisabledWeight);
        cout << "[ATC] Navigation corridor " << cyberDisabledCorridorU << " <-> "
             << cyberDisabledCorridorV << " restored to service.\n";
        cyberDisabledCorridorU = "";
        cyberDisabledCorridorV = "";
    }
    emergenciesResolved++;
    history.logCommand("COUNTER_CYBER", "CYBER_DEFENSE", currentTick);
    cout << "[ATC] Electronic countermeasures deployed! Cyber threat neutralized.\n";
    return true;
}

bool GameEngine::handleSearchMissing() {
    cout << "\nEnter Callsign of missing flight to search: ";
    string cs;
    cin >> cs;

    string result;
    bool found = radar.searchMissingFlight(cs, currentTick, result);
    cout << result << "\n";
    if (found) {
        credibility += rules.rewardMissingFound;
        emergenciesResolved++;
        history.logCommand("SEARCH_MISSING", cs, currentTick);
        return true;
    }
    return false;
}

bool GameEngine::handleUndo() {
    if (!history.canUndo()) {
        cout << "[ATC Error] Cannot undo: history is empty.\n";
        return false;
    }

    FlightSnapshot s = history.popUndoSnapshot();
    history.pushRedoSnapshot(s);

    if (s.actionType == "COUNTER_CYBER") {
        activeCyberAlert = true;
    } else {
        Flight* f = hashTable.find(s.callsign);
        if (f != nullptr) {
            f->assignedRunway = s.prevRunway;
            f->priorityLevel = s.prevPriority;
            f->currentSector = s.prevSector;
            f->status = s.prevStatus;
            f->isEmergencyResolved = s.prevEmergencyResolved;
            updateFlightRoute(f);
        }
    }

    history.logCommand("UNDO_" + s.actionType, s.callsign, currentTick);
    cout << "[ATC] Operation '" << s.actionType << "' on " << s.callsign << " undone.\n";
    return true;
}

bool GameEngine::handleRedo() {
    if (!history.canRedo()) {
        cout << "[ATC Error] Cannot redo: no redoable operations.\n";
        return false;
    }

    FlightSnapshot s = history.popRedoSnapshot();
    history.pushUndoSnapshot(s);

    if (s.actionType == "COUNTER_CYBER") {
        activeCyberAlert = false;
    } else if (s.actionType == "CLEAR_RUNWAY") {
        Flight* f = hashTable.find(s.callsign);
        if (f != nullptr) f->clearedForLanding = true;
    }

    history.logCommand("REDO_" + s.actionType, s.callsign, currentTick);
    cout << "[ATC] Operation '" << s.actionType << "' redone.\n";
    return true;
}

void GameEngine::viewManifest() const {
    registry.displayAllFlights();
    registry.displayAllCrew();
}

void GameEngine::viewRadar() {
    cout << "\n1. BFS Radar Sweep (Level-by-level)   2. DFS Airspace Reachability\n"
         << "Select Radar Mode (1 or 2): ";
    int mode = 1;
    if (cin >> mode) {
        if (mode == 1) {
            radar.bfsFrom("Control Tower");
            radar.identifyTwoHopSectors("Control Tower");
        } else {
            radar.dfsFrom("Control Tower");
            radar.runRunway27Demonstration();
        }
    } else {
        cin.clear();
        cin.ignore(1000, '\n');
    }
}

void GameEngine::viewAirspaceMap() const {
    airspace.displayAdjacencyList();
    airspace.displayAdjacencyMatrix();
}

void GameEngine::viewFastestRoute() const {
    cout << "\nEnter Source Sector: ";
    string src;
    cin.ignore();
    getline(cin, src);
    cout << "Enter Destination Runway: ";
    string dest;
    getline(cin, dest);

    RouteResult res = routeFinder.findFastestRoute(LeaderBoard::trim(src), LeaderBoard::trim(dest));
    routeFinder.printRoute(res);
}

void GameEngine::viewEmergencyQueues() const {
    landingHub.displayAllQueues();
}

void GameEngine::viewCommandHistory() const {
    history.displayHistory(10);
    history.displayUndoStack();
}

void GameEngine::viewDataStructureStats() {
    hashTable.displayStats();
    cout << "\nBST Traversal:\n";
    bst.inorderTraversal();
    cout << "\nAVL Traversal:\n";
    avl.inorderTraversal();
    radarMST.computeMST();
    radarMST.displayMST();
    radarMST.verifyConnectivity();
}

void GameEngine::advanceOneTick() {
    advanceFlights();
    escalateEmergencies();
    processScenarioEvents();
    currentTick++;
    actionsRemainingThisTurn = rules.actionsPerTurn;
    checkWinLoseConditions();
}

bool GameEngine::executeAction(int choice, const string& p1, const string& p2) {
    bool consumedAction = false;
    switch (choice) {
        case 1: { // Clear runway
            Flight* f = hashTable.find(p1);
            if (f != nullptr) {
                FlightSnapshot snap(f->callsign, f->assignedRunway, f->priorityLevel,
                                   f->currentSector, f->status, f->isEmergencyResolved,
                                   currentTick, "CLEAR_RUNWAY");
                history.pushUndoSnapshot(snap);
                f->clearedForLanding = true;
                history.logCommand("CLEAR_RUNWAY", p1, currentTick);
                consumedAction = true;
            }
            break;
        }
        case 2: { // Reroute
            Flight* f = hashTable.find(p1);
            if (f != nullptr && airspace.getSectorIndex(p2) != -1) {
                FlightSnapshot snap(f->callsign, f->assignedRunway, f->priorityLevel,
                                   f->currentSector, f->status, f->isEmergencyResolved,
                                   currentTick, "REROUTE");
                history.pushUndoSnapshot(snap);
                f->assignedRunway = p2;
                updateFlightRoute(f);
                if (f->priorityLevel == 7) {
                    f->isEmergencyResolved = true;
                    emergenciesResolved++;
                }
                history.logCommand("REROUTE", p1, currentTick);
                consumedAction = true;
            }
            break;
        }
        case 3: { // Escalate priority
            Flight* f = hashTable.find(p1);
            if (f != nullptr) {
                int p = stoi(p2);
                landingHub.removeFlightFromAll(f->callsign);
                f->priorityLevel = p;
                landingHub.insertEmergency(f);
                history.logCommand("ESCALATE_PRIORITY", p1, currentTick);
                consumedAction = true;
            }
            break;
        }
        case 4: { // Intercept hijack or war
            if (p1 == "WAR" || activeWarIncident) {
                activeWarIncident = false;
                emergenciesResolved++;
                history.logCommand("INTERCEPT_WAR", "AIR_DEFENSE", currentTick);
                consumedAction = true;
                break;
            }
            Flight* f = hashTable.find(p1);
            if (f != nullptr && f->priorityLevel == 2) {
                f->isEmergencyResolved = true;
                f->priorityLevel = 9;
                landingHub.removeFlightFromAll(f->callsign);
                landingHub.enqueueNormal(f);
                emergenciesResolved++;
                history.logCommand("INTERCEPT_HIJACK", p1, currentTick);
                consumedAction = true;
            }
            break;
        }
        case 5: { // Counter cyber
            if (activeCyberAlert) {
                activeCyberAlert = false;
                if (!cyberDisabledCorridorU.empty()) {
                    airspace.restoreCorridor(cyberDisabledCorridorU, cyberDisabledCorridorV, cyberDisabledWeight);
                    cyberDisabledCorridorU = "";
                    cyberDisabledCorridorV = "";
                }
                emergenciesResolved++;
                history.logCommand("COUNTER_CYBER", "CYBER_ALERT", currentTick);
                consumedAction = true;
            }
            break;
        }
        case 6: { // Search missing
            string res;
            if (radar.searchMissingFlight(p1, currentTick, res)) {
                credibility += rules.rewardMissingFound;
                emergenciesResolved++;
                history.logCommand("SEARCH_MISSING", p1, currentTick);
                consumedAction = true;
            }
            break;
        }
        case 7: { // Undo
            if (history.canUndo()) {
                FlightSnapshot s = history.popUndoSnapshot();
                Flight* f = hashTable.find(s.callsign);
                if (f != nullptr) {
                    f->assignedRunway = s.prevRunway;
                    f->priorityLevel = s.prevPriority;
                }
                history.logCommand("UNDO", s.callsign, currentTick);
                consumedAction = true;
            }
            break;
        }
        default:
            break;
    }

    if (consumedAction) {
        actionsRemainingThisTurn--;
        if (actionsRemainingThisTurn <= 0) {
            advanceOneTick();
        }
    }
    return consumedAction;
}

void GameEngine::runTurn() {
    displayDashboard();
    int choice = 0;
    if (!(cin >> choice)) {
        if (cin.eof()) {
            isGameRunning = false;
            terminationReason = "Input stream closed.";
            return;
        }
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "[ATC Error] Invalid menu choice. Please enter a number.\n";
        return;
    }

    bool consumedAction = false;
    switch (choice) {
        case 1:  consumedAction = handleClearRunway(); break;
        case 2:  consumedAction = handleRerouteFlight(); break;
        case 3:  consumedAction = handleEscalatePriority(); break;
        case 4:  consumedAction = handleInterceptHijack(); break;
        case 5:  consumedAction = handleCounterCyber(); break;
        case 6:  consumedAction = handleSearchMissing(); break;
        case 7:  consumedAction = handleUndo(); break;
        case 8:  consumedAction = handleRedo(); break;
        case 9:  viewManifest(); break;
        case 10: viewRadar(); break;
        case 11: viewAirspaceMap(); break;
        case 12: viewFastestRoute(); break;
        case 13: viewEmergencyQueues(); break;
        case 14: viewCommandHistory(); break;
        case 15: viewDataStructureStats(); break;
        case 16: // End Turn explicitly
            cout << "[ATC] Turn ended early by CRO. Advancing time tick...\n";
            advanceOneTick();
            return;
        case 17: { // Abort
            cout << "\n[ATC] Are you sure you want to abort the simulation? (y/n): ";
            char c = 0;
            cin >> c;
            if (c == 'y' || c == 'Y') {
                isGameRunning = false;
                gameWon = false;
                terminationReason = "Simulation manually terminated by Officer.";
                return;
            }
            break;
        }
        default:
            cout << "[ATC Error] Selection out of range. Choose between 1 and 17.\n";
            break;
    }

    if (consumedAction) {
        actionsRemainingThisTurn--;
        if (actionsRemainingThisTurn <= 0) {
            cout << "\n[ATC] Turn actions completed. Advancing simulation tick...\n";
            advanceOneTick();
        }
    }
}

void GameEngine::executeGameLoop() {
    while (isGameRunning) {
        runTurn();
    }
    displayGameOverScreen();
}

void GameEngine::displayGameOverScreen() {
    cout << "\n================================================================================\n";
    cout << "                         MISSION DEBRIEFING REPORT                              \n";
    cout << "================================================================================\n";
    if (gameWon) {
        cout << " MISSION STATUS:  *** VICTORY - AIRSPACE SECURED ***\n";
    } else {
        cout << " MISSION STATUS:  *** MISSION FAILED - SIMULATION TERMINATED ***\n";
    }
    cout << " Chief Radar Officer:   " << croName << "\n";
    cout << " Final Simulation Tick: " << currentTick << " / " << rules.winTick << "\n";
    cout << " Final Credibility:     " << credibility << " / 100\n";
    cout << " Flights Landed:        " << flightsLanded << "\n";
    cout << " Emergencies Resolved:  " << emergenciesResolved << "\n";
    cout << " Debriefing Remarks:    " << terminationReason << "\n";
    cout << "================================================================================\n";

    // Format current date/time
    time_t now = time(nullptr);
    tm* ltm = localtime(&now);
    stringstream ss;
    ss << 1900 + ltm->tm_year << "-"
       << setfill('0') << setw(2) << 1 + ltm->tm_mon << "-"
       << setw(2) << ltm->tm_mday << " "
       << setw(2) << ltm->tm_hour << ":"
       << setw(2) << ltm->tm_min << ":"
       << setw(2) << ltm->tm_sec;

    LeaderboardRecord rec;
    rec.dateTime = ss.str();
    rec.croName = croName;
    rec.finalTick = currentTick;
    rec.credibility = credibility;
    rec.flightsLanded = flightsLanded;
    rec.emergenciesResolved = emergenciesResolved;
    rec.outcome = gameWon ? "VICTORY" : "DEFEAT";
    rec.reason = terminationReason;

    LeaderBoard::appendLeaderboard("data/leaderboard.txt", rec);
    cout << "[Persistence] Debriefing record saved to leaderboard.txt.\n\n";
}
