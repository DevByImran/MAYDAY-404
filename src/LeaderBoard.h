#ifndef LEADER_BOARD_H
#define LEADER_BOARD_H

#include <string>
#include <iostream>
#include "Flight.h"
#include "Crew.h"
#include "Registration.h"
#include "HashTable.h"
#include "BSTflight.h"
#include "AVLflight.h"
#include "EmergencyLanding.h"
#include "SpaceGraph.h"

using namespace std;

const int MAX_SCENARIO_EVENTS = 40;
const int MAX_LEADERBOARD_RECORDS = 50;

struct GameRules {
    int startingCredibility;
    int maxTicks;
    int actionsPerTurn;
    int winTick;

    int deadlineWar;
    int deadlineHijack;
    int deadlineCyber;
    int deadlineMissing;
    int deadlineFuel;
    int deadlineMedical;
    int deadlineWeather;
    int deadlineMechanical;

    int rewardLanding;
    int rewardMissingFound;
    int penaltyCrash;
    int penaltyMedical;
    int penaltyWeatherDivert;
    int penaltyCyberDisable;
    int penaltyBreach;

    GameRules()
        : startingCredibility(100), maxTicks(30), actionsPerTurn(2), winTick(30),
          deadlineWar(2), deadlineHijack(3), deadlineCyber(3), deadlineMissing(4),
          deadlineFuel(5), deadlineMedical(4), deadlineWeather(4), deadlineMechanical(5),
          rewardLanding(2), rewardMissingFound(5), penaltyCrash(30), penaltyMedical(15),
          penaltyWeatherDivert(10), penaltyCyberDisable(20), penaltyBreach(25) {}
};

enum EventType {
    EVENT_SPAWN,
    EVENT_CYBER,
    EVENT_MISSING,
    EVENT_WEATHER,
    EVENT_WAR,
    EVENT_MECHANICAL
};

struct ScenarioEvent {
    int tick;
    EventType type;
    Flight spawnFlight;
    string target;
    string description;
    bool triggered;

    ScenarioEvent() : tick(0), type(EVENT_SPAWN), target(""), description(""), triggered(false) {}
};

struct LeaderboardRecord {
    string dateTime;
    string croName;
    int finalTick;
    int credibility;
    int flightsLanded;
    int emergenciesResolved;
    string outcome; // VICTORY or DEFEAT
    string reason;

    LeaderboardRecord()
        : dateTime(""), croName(""), finalTick(0), credibility(0),
          flightsLanded(0), emergenciesResolved(0), outcome(""), reason("") {}
};

class LeaderBoard {
public:
    static string trim(const string& str);

    // Loaders for assignment input files
    static int loadFlights(const string& filename, Registration& reg, HashTable& ht,
                           BSTflight& bst, AVLflight& avl, EmergencyLanding& el);
    static int loadCrew(const string& filename, Registration& reg);
    static int loadAirspace(const string& filename, SpaceGraph& graph);
    static bool loadRules(const string& filename, GameRules& rules);
    static int loadScenario(const string& filename, ScenarioEvent events[], int maxEvents);

    // Text display helpers
    static void displayFileContent(const string& filename);

    // Leaderboard file persistence
    static bool appendLeaderboard(const string& filename, const LeaderboardRecord& record);
    static void displayLeaderboard(const string& filename);
};

#endif // LEADER_BOARD_H
