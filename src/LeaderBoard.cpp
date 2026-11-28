#include "LeaderBoard.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

string LeaderBoard::trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

int LeaderBoard::loadFlights(const string& filename, Registration& reg, HashTable& ht,
                            BSTflight& bst, AVLflight& avl, EmergencyLanding& el) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "[ATC Error] Could not open flights file: " << filename << "\n";
        return 0;
    }

    string line;
    int loadedCount = 0;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        stringstream ss(line);
        string callsign, aircraftType, airline, origin, destination;
        string passengersStr, fuelStr, currentSector, assignedRunway;
        string arrTickStr, priorityStr, status;

        if (!getline(ss, callsign, ',')) continue;
        if (!getline(ss, aircraftType, ',')) continue;
        if (!getline(ss, airline, ',')) continue;
        if (!getline(ss, origin, ',')) continue;
        if (!getline(ss, destination, ',')) continue;
        if (!getline(ss, passengersStr, ',')) continue;
        if (!getline(ss, fuelStr, ',')) continue;
        if (!getline(ss, currentSector, ',')) continue;
        if (!getline(ss, assignedRunway, ',')) continue;
        if (!getline(ss, arrTickStr, ',')) continue;
        if (!getline(ss, priorityStr, ',')) continue;
        if (!getline(ss, status, ',')) {
            status = "ACTIVE";
        }

        callsign = trim(callsign);
        aircraftType = trim(aircraftType);
        airline = trim(airline);
        origin = trim(origin);
        destination = trim(destination);
        currentSector = trim(currentSector);
        assignedRunway = trim(assignedRunway);
        status = trim(status);

        int passengers = 150;
        int fuel = 15;
        int arrTick = 1;
        int priority = 9;

        try { passengers = stoi(trim(passengersStr)); } catch (...) {}
        try { fuel = stoi(trim(fuelStr)); } catch (...) {}
        try { arrTick = stoi(trim(arrTickStr)); } catch (...) {}
        try { priority = stoi(trim(priorityStr)); } catch (...) {}

        // Prevent duplicate callsigns
        if (ht.find(callsign) != nullptr) {
            cout << "[ATC Warning] Skipping duplicate flight: " << callsign << "\n";
            continue;
        }

        Flight* newFlight = new Flight(callsign, aircraftType, airline, origin, destination,
                                       passengers, fuel, currentSector, assignedRunway,
                                       arrTick, priority, status);

        // Register in primary owner
        reg.addFlight(newFlight);

        // Register in fast lookup structures
        ht.insert(callsign, newFlight);
        bst.insert(newFlight);
        avl.insert(newFlight);

        // Register in landing dispatch
        if (priority == 9) {
            el.enqueueNormal(newFlight);
        } else {
            el.insertEmergency(newFlight);
        }

        loadedCount++;
    }

    file.close();
    return loadedCount;
}

int LeaderBoard::loadCrew(const string& filename, Registration& reg) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "[ATC Error] Could not open crew file: " << filename << "\n";
        return 0;
    }

    string line;
    int loaded = 0;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        stringstream ss(line);
        string id, name, role, callsign;
        if (getline(ss, id, ',') && getline(ss, name, ',') &&
            getline(ss, role, ',') && getline(ss, callsign, ',')) {
            Crew c(trim(id), trim(name), trim(role), trim(callsign));
            reg.addCrew(c);
            loaded++;
        }
    }
    file.close();
    return loaded;
}

int LeaderBoard::loadAirspace(const string& filename, SpaceGraph& graph) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "[ATC Error] Could not open airspace file: " << filename << ". Loading defaults.\n";
        graph.initializeDefaultAirspace();
        return graph.getSectorCount();
    }

    string line;
    int sectorsLoaded = 0;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        if (line.rfind("SECTOR:", 0) == 0) {
            string data = line.substr(7);
            stringstream ss(data);
            string name, isRunwayStr, desc;
            if (getline(ss, name, ',') && getline(ss, isRunwayStr, ',')) {
                if (!getline(ss, desc, ',')) desc = "";
                bool isRunway = (trim(isRunwayStr) == "1");
                graph.addSector(trim(name), isRunway, trim(desc));
                sectorsLoaded++;
            }
        } else if (line.rfind("CORRIDOR:", 0) == 0) {
            string data = line.substr(9);
            stringstream ss(data);
            string from, to, weightStr;
            if (getline(ss, from, ',') && getline(ss, to, ',') && getline(ss, weightStr, ',')) {
                int w = 3;
                try { w = stoi(trim(weightStr)); } catch (...) {}
                graph.addCorridor(trim(from), trim(to), w);
            }
        }
    }

    file.close();
    if (sectorsLoaded == 0) {
        graph.initializeDefaultAirspace();
        sectorsLoaded = graph.getSectorCount();
    }
    return sectorsLoaded;
}

bool LeaderBoard::loadRules(const string& filename, GameRules& rules) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "[ATC Warning] Could not open rules file: " << filename << ". Using default rules.\n";
        return false;
    }

    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        size_t eqPos = line.find('=');
        if (eqPos == string::npos) continue;

        string key = trim(line.substr(0, eqPos));
        string valStr = trim(line.substr(eqPos + 1));
        int val = 0;
        try { val = stoi(valStr); } catch (...) { continue; }

        if (key == "STARTING_CREDIBILITY") rules.startingCredibility = val;
        else if (key == "MAX_TICKS") rules.maxTicks = val;
        else if (key == "ACTIONS_PER_TURN") rules.actionsPerTurn = val;
        else if (key == "WIN_TICK") rules.winTick = val;
        else if (key == "DEADLINE_WAR") rules.deadlineWar = val;
        else if (key == "DEADLINE_HIJACK") rules.deadlineHijack = val;
        else if (key == "DEADLINE_CYBER") rules.deadlineCyber = val;
        else if (key == "DEADLINE_MISSING") rules.deadlineMissing = val;
        else if (key == "DEADLINE_FUEL") rules.deadlineFuel = val;
        else if (key == "DEADLINE_MEDICAL") rules.deadlineMedical = val;
        else if (key == "DEADLINE_WEATHER") rules.deadlineWeather = val;
        else if (key == "DEADLINE_MECHANICAL") rules.deadlineMechanical = val;
        else if (key == "REWARD_LANDING") rules.rewardLanding = val;
        else if (key == "REWARD_MISSING_FOUND") rules.rewardMissingFound = val;
        else if (key == "PENALTY_CRASH") rules.penaltyCrash = val;
        else if (key == "PENALTY_MEDICAL") rules.penaltyMedical = val;
        else if (key == "PENALTY_WEATHER_DIVERT") rules.penaltyWeatherDivert = val;
        else if (key == "PENALTY_CYBER_DISABLE") rules.penaltyCyberDisable = val;
        else if (key == "PENALTY_BREACH") rules.penaltyBreach = val;
    }

    file.close();
    return true;
}

int LeaderBoard::loadScenario(const string& filename, ScenarioEvent events[], int maxEvents) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "[ATC Error] Could not open scenario file: " << filename << "\n";
        return 0;
    }

    string line;
    int count = 0;
    while (getline(file, line) && count < maxEvents) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        if (line.rfind("SPAWN:", 0) == 0) {
            string data = line.substr(6);
            stringstream ss(data);
            string tickStr, cs, type, al, orig, dest, passStr, fuelStr, sector, runway, prioStr;

            if (getline(ss, tickStr, ',') && getline(ss, cs, ',') &&
                getline(ss, type, ',') && getline(ss, al, ',') &&
                getline(ss, orig, ',') && getline(ss, dest, ',') &&
                getline(ss, passStr, ',') && getline(ss, fuelStr, ',') &&
                getline(ss, sector, ',') && getline(ss, runway, ',') &&
                getline(ss, prioStr, ',')) {

                int tick = stoi(trim(tickStr));
                int pass = stoi(trim(passStr));
                int fuel = stoi(trim(fuelStr));
                int prio = stoi(trim(prioStr));

                events[count].tick = tick;
                events[count].type = EVENT_SPAWN;
                events[count].target = trim(cs);
                events[count].description = "New flight arrival inbound";
                events[count].triggered = false;
                events[count].spawnFlight = Flight(trim(cs), trim(type), trim(al), trim(orig),
                                                  trim(dest), pass, fuel, trim(sector),
                                                  trim(runway), tick, prio, "ACTIVE");
                count++;
            }
        } else if (line.rfind("EVENT:", 0) == 0) {
            string data = line.substr(6);
            stringstream ss(data);
            string tickStr, typeStr, target, desc;
            if (getline(ss, tickStr, ',') && getline(ss, typeStr, ',') &&
                getline(ss, target, ',') && getline(ss, desc, ',')) {

                int tick = stoi(trim(tickStr));
                string typeUpper = trim(typeStr);
                EventType evType = EVENT_CYBER;
                if (typeUpper == "CYBER") evType = EVENT_CYBER;
                else if (typeUpper == "MISSING") evType = EVENT_MISSING;
                else if (typeUpper == "WEATHER") evType = EVENT_WEATHER;
                else if (typeUpper == "WAR") evType = EVENT_WAR;
                else if (typeUpper == "MECHANICAL") evType = EVENT_MECHANICAL;

                events[count].tick = tick;
                events[count].type = evType;
                events[count].target = trim(target);
                events[count].description = trim(desc);
                events[count].triggered = false;
                count++;
            }
        }
    }

    file.close();
    return count;
}

void LeaderBoard::displayFileContent(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "[ATC Error] File not found: " << filename << "\n";
        return;
    }
    string line;
    while (getline(file, line)) {
        cout << line << "\n";
    }
    file.close();
}

bool LeaderBoard::appendLeaderboard(const string& filename, const LeaderboardRecord& record) {
    ofstream file(filename, ios::app);
    if (!file.is_open()) {
        cout << "[ATC Error] Could not open leaderboard file for appending: " << filename << "\n";
        return false;
    }

    file << record.dateTime << ", "
         << record.croName << ", "
         << record.finalTick << ", "
         << record.credibility << ", "
         << record.flightsLanded << ", "
         << record.emergenciesResolved << ", "
         << record.outcome << ", "
         << record.reason << "\n";

    file.close();
    return true;
}

void LeaderBoard::displayLeaderboard(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "[ATC Error] Leaderboard file not found: " << filename << "\n";
        return;
    }

    cout << "\n========================================================================================================\n";
    cout << "                                  ATC SIMULATOR - HALL OF FAME LEADERBOARD                              \n";
    cout << "========================================================================================================\n";
    cout << left << setw(20) << "TIMESTAMP"
         << setw(16) << "CRO NAME"
         << setw(6) << "TICK"
         << setw(6) << "SCORE"
         << setw(8) << "LANDED"
         << setw(8) << "RESOLV"
         << setw(10) << "OUTCOME"
         << "REMARKS" << "\n";
    cout << "--------------------------------------------------------------------------------------------------------\n";

    string line;
    int recordCount = 0;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        stringstream ss(line);
        string dt, name, tickStr, credStr, landedStr, resStr, outcome, reason;
        if (getline(ss, dt, ',') && getline(ss, name, ',') &&
            getline(ss, tickStr, ',') && getline(ss, credStr, ',') &&
            getline(ss, landedStr, ',') && getline(ss, resStr, ',') &&
            getline(ss, outcome, ',') && getline(ss, reason, ',')) {

            cout << left << setw(20) << trim(dt)
                 << setw(16) << trim(name)
                 << setw(6) << trim(tickStr)
                 << setw(6) << trim(credStr)
                 << setw(8) << trim(landedStr)
                 << setw(8) << trim(resStr)
                 << setw(10) << trim(outcome)
                 << trim(reason) << "\n";
            recordCount++;
        }
    }

    if (recordCount == 0) {
        cout << "No past simulation records found.\n";
    }
    cout << "========================================================================================================\n\n";
    file.close();
}
