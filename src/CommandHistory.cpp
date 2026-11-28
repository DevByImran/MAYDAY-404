#include "CommandHistory.h"
#include <iomanip>

using namespace std;

// ==========================================
// SnapshotStack Implementation
// ==========================================

SnapshotStack::SnapshotStack(int initialCap)
    : capacity(initialCap), topIdx(-1) {
    stackArray = new FlightSnapshot[capacity];
}

SnapshotStack::~SnapshotStack() {
    delete[] stackArray;
}

void SnapshotStack::resize() {
    int newCap = capacity * 2;
    FlightSnapshot* newArr = new FlightSnapshot[newCap];
    for (int i = 0; i <= topIdx; ++i) {
        newArr[i] = stackArray[i];
    }
    delete[] stackArray;
    stackArray = newArr;
    capacity = newCap;
}

void SnapshotStack::push(const FlightSnapshot& s) {
    if (topIdx + 1 >= capacity) {
        resize();
    }
    stackArray[++topIdx] = s;
}

FlightSnapshot SnapshotStack::pop() {
    if (isEmpty()) {
        return FlightSnapshot();
    }
    return stackArray[topIdx--];
}

FlightSnapshot SnapshotStack::top() const {
    if (isEmpty()) {
        return FlightSnapshot();
    }
    return stackArray[topIdx];
}

void SnapshotStack::clear() {
    topIdx = -1;
}

void SnapshotStack::display(const string& stackName) const {
    cout << "\n--------------------------------------------------\n";
    cout << "             " << stackName << " (LIFO)            \n";
    cout << "--------------------------------------------------\n";
    if (isEmpty()) {
        cout << "Stack is empty.\n";
        return;
    }
    cout << left << setw(6) << "LEVEL"
         << setw(10) << "ACTION"
         << setw(10) << "CALLSIGN"
         << setw(8) << "TICK"
         << setw(12) << "PREV_SECTOR"
         << setw(12) << "PREV_RUNWAY" << "\n";
    for (int i = topIdx; i >= 0; --i) {
        const FlightSnapshot& s = stackArray[i];
        cout << left << setw(6) << (topIdx - i + 1)
             << setw(10) << s.actionType
             << setw(10) << s.callsign
             << setw(8) << s.tick
             << setw(12) << s.prevSector
             << setw(12) << s.prevRunway << "\n";
    }
    cout << "Total Snapshots: " << (topIdx + 1) << "\n";
}

// ==========================================
// CommandHistory Implementation
// ==========================================

CommandHistory::CommandHistory()
    : head(nullptr), tail(nullptr), historyCount(0) {}

CommandHistory::~CommandHistory() {
    clearAll();
}

void CommandHistory::logCommand(const string& actionName, const string& flightCallsign, int tickNumber) {
    CommandNode* newNode = new CommandNode(actionName, flightCallsign, tickNumber);
    if (head == nullptr) {
        head = newNode;
        tail = newNode;
    } else {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }
    historyCount++;
}

void CommandHistory::displayHistory(int count) const {
    cout << "\n================================================================================\n";
    cout << "                    COMMAND HISTORY (REVERSE CHRONOLOGICAL)                     \n";
    cout << "================================================================================\n";
    if (tail == nullptr) {
        cout << "No commands logged yet.\n";
        return;
    }
    cout << left << setw(6) << "NO."
         << setw(8) << "TICK"
         << setw(24) << "ACTION"
         << setw(12) << "CALLSIGN" << "\n";
    cout << "--------------------------------------------------------------------------------\n";

    CommandNode* curr = tail;
    int displayed = 0;
    while (curr != nullptr && displayed < count) {
        cout << left << setw(6) << (displayed + 1)
             << setw(8) << curr->tickNumber
             << setw(24) << curr->actionName
             << setw(12) << curr->flightCallsign << "\n";
        curr = curr->prev;
        displayed++;
    }
    cout << "================================================================================\n";
    cout << "Showing " << displayed << " of " << historyCount << " logged commands.\n\n";
}

void CommandHistory::displayLastCommand() const {
    if (tail == nullptr) {
        cout << "[ATC] No commands executed yet.\n";
    } else {
        cout << "[ATC Last Action] Tick " << tail->tickNumber
             << ": " << tail->actionName << " on Flight " << tail->flightCallsign << "\n";
    }
}

void CommandHistory::pushUndoSnapshot(const FlightSnapshot& snapshot) {
    undoStack.push(snapshot);
}

FlightSnapshot CommandHistory::popUndoSnapshot() {
    return undoStack.pop();
}

void CommandHistory::pushRedoSnapshot(const FlightSnapshot& snapshot) {
    redoStack.push(snapshot);
}

FlightSnapshot CommandHistory::popRedoSnapshot() {
    return redoStack.pop();
}

void CommandHistory::clearRedoStack() {
    redoStack.clear();
}

void CommandHistory::displayUndoStack() const {
    undoStack.display("UNDO STACK");
}

void CommandHistory::clearAll() {
    CommandNode* curr = head;
    while (curr != nullptr) {
        CommandNode* nextNode = curr->next;
        delete curr;
        curr = nextNode;
    }
    head = nullptr;
    tail = nullptr;
    historyCount = 0;
    undoStack.clear();
    redoStack.clear();
}
