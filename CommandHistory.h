#ifndef COMMAND_HISTORY_H
#define COMMAND_HISTORY_H

#include <string>
#include <iostream>

using namespace std;

// Node for Doubly Linked List History
struct CommandNode {
    string actionName;
    string flightCallsign;
    int tickNumber;
    CommandNode* prev;
    CommandNode* next;

    CommandNode(const string& act, const string& cs, int tick)
        : actionName(act), flightCallsign(cs), tickNumber(tick),
          prev(nullptr), next(nullptr) {}
};

// Snapshot structure for restoring flight state
struct FlightSnapshot {
    string callsign;
    string prevRunway;
    int prevPriority;
    string prevSector;
    string prevStatus;
    bool prevEmergencyResolved;
    int tick;
    string actionType;
    int credibilityChange; // Reversible credibility impact

    FlightSnapshot()
        : callsign(""), prevRunway(""), prevPriority(9),
          prevSector(""), prevStatus(""), prevEmergencyResolved(false),
          tick(0), actionType(""), credibilityChange(0) {}

    FlightSnapshot(const string& cs, const string& runway, int priority,
                   const string& sector, const string& status, bool resolved,
                   int t, const string& action, int credChange = 0)
        : callsign(cs), prevRunway(runway), prevPriority(priority),
          prevSector(sector), prevStatus(status), prevEmergencyResolved(resolved),
          tick(t), actionType(action), credibilityChange(credChange) {}
};

// Custom Stack for Undo / Redo snapshots (Dynamic Array, no vectors)
class SnapshotStack {
private:
    FlightSnapshot* stackArray;
    int capacity;
    int topIdx;

    void resize();

public:
    explicit SnapshotStack(int initialCap = 32);
    ~SnapshotStack();

    SnapshotStack(const SnapshotStack&) = delete;
    SnapshotStack& operator=(const SnapshotStack&) = delete;

    void push(const FlightSnapshot& s);
    FlightSnapshot pop();
    FlightSnapshot top() const;
    bool isEmpty() const { return topIdx == -1; }
    int getCount() const { return topIdx + 1; }
    void clear();
    void display(const string& stackName) const;
};

// Module 3 Orchestrator
class CommandHistory {
private:
    // Doubly Linked List for History
    CommandNode* head;
    CommandNode* tail;
    int historyCount;

    // Custom Stacks for Undo and Redo
    SnapshotStack undoStack;
    SnapshotStack redoStack;

public:
    CommandHistory();
    ~CommandHistory();

    CommandHistory(const CommandHistory&) = delete;
    CommandHistory& operator=(const CommandHistory&) = delete;

    // Doubly linked list operations
    void logCommand(const string& actionName, const string& flightCallsign, int tickNumber);
    void displayHistory(int count = 10) const;
    void displayLastCommand() const;
    int getHistoryCount() const { return historyCount; }

    // Undo / Redo operations
    void pushUndoSnapshot(const FlightSnapshot& snapshot);
    bool canUndo() const { return !undoStack.isEmpty(); }
    FlightSnapshot popUndoSnapshot();

    void pushRedoSnapshot(const FlightSnapshot& snapshot);
    bool canRedo() const { return !redoStack.isEmpty(); }
    FlightSnapshot popRedoSnapshot();

    void clearRedoStack();
    void displayUndoStack() const;
    void clearAll();
};

#endif // COMMAND_HISTORY_H
