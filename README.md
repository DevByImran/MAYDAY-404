# Air Traffic Control (ATC) Simulator Game
### Data Structures & Algorithms University Semester Project

---

## 1. Project Introduction

The **Air Traffic Control (ATC) Simulator Game** is a complete, console-based simulation developed in **pure C++17**. It models an airport operations control room where the player assumes the critical role of **Chief Radar Officer (CRO)**. Over a 30-tick operational window (representing 15 simulated minutes), the CRO manages airspace traffic, directs normal and emergency arrivals, responds to dynamic security crises (hijackings, cyber intrusions, hostile military intrusions, search-and-rescue missing flights), and navigates flights through corridor graphs to designated runways.

This project demonstrates the practical integration of core computer science data structures and algorithms implemented **entirely from scratch** without using standard STL container replacements (`std::list`, `std::stack`, `std::queue`, `std::priority_queue`, `std::map`, or `std::unordered_map`). In accordance with project instructions, all code uses standard arrays and pointers rather than `std::vector`, and includes `using namespace std;`.

---

## 2. Game Objective and Simulation Rules

- **Simulation Duration**: Exactly 30 ticks (1 tick = 30 simulated seconds).
- **Turn Actions**: The Chief Radar Officer receives up to **2 actions per turn**.
- **Free Operations**: Viewing radar, checking airspace map, viewing manifests, calculating shortest paths, and reviewing statistics consume **0 actions**.
- **Action-Consuming Operations**: Clearing flights for runway, rerouting, priority escalation, hijack intercepts, cyber defense countermeasures, missing plane radar sweeps, and undo/redo consume **1 action**.
- **Turn Advance**: After 2 actions, or when the player chooses "End Turn", the simulation automatically advances one tick.
- **Flight Movement & Fuel**: Each tick, active aircraft advance along their Dijkstra-calculated corridor routes toward their assigned runways. Remaining fuel decreases by 1 every tick. Fuel exhaustion causes a fatal crash.
- **Emergency Priority Hierarchy**:
  1. **War** (Hostile combat intrusion): Must be scrambled and neutralized within 2 ticks.
  2. **Hijack**: Must be intercepted by tactical forces within 3 ticks.
  3. **Cyber Alert**: Must be countered within 3 ticks or a corridor is jammed; after 4 ticks, the control tower is disabled.
  4. **Missing Flight**: Offline search-and-rescue transponder search; if deadline expires, triggers a Restricted Zone security breach.
  5. **Fuel Emergency**: Aircraft with critical fuel (<= 3); prioritized for emergency arrestor runways.
  6. **Medical Emergency**: Must land within 4 ticks or passenger casualties occur.
  7. **Weather Emergency**: Severe microburst windshear; must be rerouted to an alternate runway within 4 ticks.
  8. **Mechanical Failure**: Hydraulic/flight control loss; must land within 5 ticks.
  9. **Normal Flight**: Standard scheduled commercial arrival.
- **Win Condition**: Reach tick 30 without any fatal losing condition.
- **Lose Conditions**:
  - Airport credibility reaches 0.
  - Any unhandled fatal crash (fuel exhaustion, mechanical structural failure, severe storm windshear).
  - Hijack deadline expiration (hijackers seize the aircraft).
  - Cyber attack disables central tower operations.
  - War intrusion deadline expiration (hostile combat aircraft breaches perimeter).

---

## 3. Custom Data Structures Summary

| Data Structure | Module | File Location | Purpose & Practical Application |
|---|---|---|---|
| **Singly Linked List** | Module 1: `Registration` | `include/Registration.h`, `src/Registration.cpp` | Primary owner of active flight records. Maintained with head and tail pointers for $O(1)$ tail insertions. |
| **Hash Table (Linear Probing)** | Module 1: `HashTable` | `include/HashTable.h`, `src/HashTable.cpp` | Prime capacity (101) open addressing table with linear probing and tombstone deletion. Provides average $O(1)$ callsign lookup. |
| **Circular Queue (FIFO)** | Module 2: `EmergencyLanding` | `include/EmergencyLanding.h`, `src/EmergencyLanding.cpp` | Custom circular dynamic array queue for normal commercial arrivals. $O(1)$ enqueue and dequeue. |
| **Array-Based Min-Heap** | Module 2: `EmergencyLanding` | `include/EmergencyLanding.h`, `src/EmergencyLanding.cpp` | Priority queue organizing emergencies by urgency (1-9) with arrival-tick tie-breaking. $O(\log N)$ extraction. |
| **Doubly Linked List** | Module 3: `CommandHistory` | `include/CommandHistory.h`, `src/CommandHistory.cpp` | Audit log of all player actions, traversed backward from tail for reverse-chronological history display. |
| **Custom Stack (LIFO)** | Module 3: `CommandHistory` | `include/CommandHistory.h`, `src/CommandHistory.cpp` | Undo and Redo stacks storing flight state snapshots for state restoration. |
| **Binary Search Tree** | Module 4: `BSTflight` | `include/BSTflight.h`, `src/BSTflight.cpp` | Binary search tree keyed alphabetically by flight callsign. Supports insertion, search, and node deletion. |
| **Self-Balancing AVL Tree** | Module 5: `AVLflight` | `include/AVLflight.h`, `src/AVLflight.cpp` | Height-balanced binary search tree maintaining balance factor in $\{-1, 0, +1\}$ via LL, RR, LR, and RL rotations. |
| **Graph (List & Matrix)** | Module 6: `SpaceGraph` | `include/SpaceGraph.h`, `src/SpaceGraph.cpp` | Synchronized Adjacency List (for fast neighbor iteration) and Adjacency Matrix (for $O(1)$ edge-weight lookup). |
| **Breadth-First Search** | Module 7: `RadarSweep` | `include/RadarSweep.h`, `src/RadarSweep.cpp` | Level-by-level radar sweep using custom queue; identifies all sectors within 2 hops of Control Tower. |
| **Depth-First Search** | Module 7: `RadarSweep` | `include/RadarSweep.h`, `src/RadarSweep.cpp` | Airspace sector reachability and network connectivity testing. |
| **Dijkstra's Algorithm** | Module 8: `RouteFinder` | `include/RouteFinder.h`, `src/RouteFinder.cpp` | Shortest path flight pathfinding using custom min-heap and parent-pointer path reconstruction. |
| **Minimum Spanning Tree** | Module 9: `RadarMST` | `include/RadarMST.h`, `src/RadarMST.cpp` | Kruskal's algorithm with custom edge QuickSort and Disjoint Set (Union-Find) with path compression and rank. |

---

## 4. Architecture and File Structure

```
ATC_Simulator/
├── CMakeLists.txt              # Standard CMake build definition
├── Makefile                    # Make build system with individual unit test targets
├── README.md                   # Comprehensive project documentation
├── DATA_STRUCTURES_EXPLANATION.md # Detailed viva and algorithmic explanation
├── flightsimulator.cpp         # Root compilation wrapper / alias
├── main.cpp                    # Interactive console application and diagnostic CLI
├── include/
│   ├── Flight.h                # Core flight model and priority definitions
│   ├── Crew.h                  # Flight crew model
│   ├── Registration.h          # Module 1: Singly linked list & crew registry
│   ├── HashTable.h             # Module 1: Open-addressing hash table
│   ├── EmergencyLanding.h      # Module 2: FIFO queue & emergency min-heap
│   ├── CommandHistory.h        # Module 3: Doubly linked list & undo/redo stacks
│   ├── BSTflight.h             # Module 4: Binary search tree
│   ├── AVLflight.h             # Module 5: Self-balancing AVL tree
│   ├── SpaceGraph.h            # Module 6: Synchronized list & matrix graph
│   ├── RadarSweep.h            # Module 7: BFS, DFS & missing flight manager
│   ├── RouteFinder.h           # Module 8: Dijkstra shortest path engine
│   ├── RadarMST.h              # Module 9: Kruskal MST & Disjoint Set
│   ├── LeaderBoard.h           # Module 11: File parser & leaderboard persistence
│   └── GameEngine.h            # Module 10: Master game loop & integration
├── src/
│   ├── Registration.cpp
│   ├── HashTable.cpp
│   ├── EmergencyLanding.cpp
│   ├── CommandHistory.cpp
│   ├── BSTflight.cpp
│   ├── AVLflight.cpp
│   ├── SpaceGraph.cpp
│   ├── RadarSweep.cpp
│   ├── RouteFinder.cpp
│   ├── RadarMST.cpp
│   ├── LeaderBoard.cpp
│   └── GameEngine.cpp
├── data/
│   ├── flights.txt             # Initial scheduled flights
│   ├── crew.txt                # Flight crew records
│   ├── airspace.txt            # Airspace sectors, runways, and weighted corridors
│   ├── scenario.txt            # Dynamic arrival and emergency event schedule
│   ├── rules.txt               # Configurable game constants, deadlines, rewards/penalties
│   ├── about.txt               # Project and developer metadata
│   ├── gamemenu.txt            # Main menu layout
│   └── leaderboard.txt         # Persistent history of simulation runs
└── tests/
    ├── test_stage1.cpp         # Singly linked list, hash table, crew
    ├── test_stage2.cpp         # FIFO queue, min-heap, dispatch rule
    ├── test_stage3.cpp         # Doubly linked history, undo/redo stacks
    ├── test_stage4.cpp         # BST and AVL insertion, search, rotations, deletion
    ├── test_stage5_6.cpp       # Graph list/matrix, BFS, DFS, reachability
    ├── test_stage7.cpp         # Dijkstra route finder, Kruskal MST, bridge tests
    ├── test_stage8.cpp         # File loaders, leaderboard persistence
    └── test_integration.cpp    # 30-tick full simulation integration test
```

---

## 5. Input File Formats

All text configuration files in `data/` support blank lines and `#` comment headers:

1. `flights.txt`:
   ```
   CALLSIGN, AIRCRAFT_TYPE, AIRLINE, ORIGIN, DESTINATION, PASSENGERS, FUEL, CURRENT_SECTOR, ASSIGNED_RUNWAY, ARRIVAL_TICK, PRIORITY_LEVEL, STATUS
   ```
2. `crew.txt`:
   ```
   EMPLOYEE_ID, NAME, ROLE, ASSIGNED_CALLSIGN
   ```
3. `airspace.txt`:
   ```
   SECTOR: <SectorName>, <IsRunway: 0 or 1>, <Description>
   CORRIDOR: <Sector1>, <Sector2>, <TravelTimeWeight>
   ```
4. `scenario.txt`:
   ```
   SPAWN: TICK, CALLSIGN, TYPE, AIRLINE, ORIGIN, DEST, PASSENGERS, FUEL, SECTOR, RUNWAY, PRIORITY
   EVENT: TICK, TYPE, TARGET_CALLSIGN_OR_SECTOR, DESCRIPTION
   ```
5. `rules.txt`:
   Key-value pairs (`KEY=VALUE`) configuring deadlines, starting credibility, actions per turn, and penalties.

---

## 6. Compilation Instructions

### Standard Compilation (Using Make)

To compile the complete application:
```bash
make
```
This builds the binary `atc_sim`.

To run all automated unit and integration tests:
```bash
make run_tests
```

### Direct Compilation with g++

```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp main.cpp -o atc_sim
```

Or using the root `flightsimulator.cpp` wrapper:
```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp flightsimulator.cpp -o atc_sim
```

### Compilation with CMake

```bash
mkdir -p build && cd build
cmake ..
cmake --build .
```

---

## 7. How to Play and Command Line Options

### Launching the Game
```bash
./atc_sim
```

### Evaluation & Demonstration Flags
- `./atc_sim --demo` or `-d`: Runs the required **Module 5 BST vs. AVL self-balancing comparison** (inserts 15 alphabetical callsigns, showing BST height 15 vs. AVL height 4).
- `./atc_sim --mst` or `-m`: Runs the **Module 9 Minimum Spanning Tree** analysis and connectivity bridge verification.
- `./atc_sim --test` or `-t`: Executes the **Automated System Diagnostic Test Suite** covering all 11 modules.

### Turn Commands

```
1. Clear Flight for Runway   (Action 1): Clears holding aircraft for immediate landing.
2. Reroute Flight            (Action 1): Assigns alternate runway (used to avoid weather windshears).
3. Escalate Priority         (Action 1): Changes flight urgency level (1 - 8).
4. Intercept Hijack / War    (Action 1): Scrambles security forces to intercept hijackers or hostile military intruders.
5. Counter Cyber Alert       (Action 1): Deploys electronic countermeasures to neutralize network attacks and restore jammed corridors.
6. Search Missing Flight     (Action 1): Executes BFS sweep to locate transponder-lost aircraft.
7. Undo Last Command         (Action 1): Restores flight state prior to previous command.
8. Redo Command              (Action 1): Re-applies undone command.
9. View Flight Manifest      (Free): Displays active flight registry and crew details.
10. View Radar               (Free): Runs live BFS level-by-level sweeps or DFS traversals.
11. View Airspace Map        (Free): Displays adjacency list and adjacency matrix representations.
12. View Fastest Route       (Free): Interactively calculates Dijkstra shortest route between any sector and runway.
13. View Queues/Heap         (Free): Inspects FIFO normal queue and emergency min-heap.
14. View Command History     (Free): Displays reverse-chronological command audit log.
15. View Data Structures     (Free): Displays hash table statistics, tree traversals, and MST analysis.
16. End Turn / Advance Tick  (Free): Concludes turn and advances simulation by one tick.
17. Abort Simulation        (Free): Exits current game back to main menu.
```

---

## 8. Documented Assignment Ambiguities

1. **Runway 27 Reachability Query**:
   - Section 11 of the assignment requests checking whether Runway 27 is reachable from Alpha Tree.
   - However, Table 1 specifies only Runways 12, 04, 17, and 09.
   - In accordance with **Rule 1** ("Do not silently add it as a real runway; treat this as an assignment inconsistency, document it, and make the demonstration configurable"), Runway 27 is intentionally not included in the default operational airport layout.
   - The DFS reachability test correctly diagnoses Runway 27 as non-existent / unreachable, preventing application crashes.

2. **Emergency Dispatch Rule**:
   - Section 6 states: *"If the normal landing queue is not empty, process its front flight. Otherwise, extract the highest-priority emergency from the heap."*
   - This exact dispatch rule has been faithfully implemented in `EmergencyLanding::dispatchNext()`.
