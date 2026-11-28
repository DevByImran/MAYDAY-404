#include <iostream>
#include <cassert>
#include "CommandHistory.h"

using namespace std;

void testDoublyLinkedListHistory() {
    cout << "[TEST] Running Doubly Linked List Command History tests...\n";
    CommandHistory ch;
    assert(ch.getHistoryCount() == 0);

    ch.logCommand("CLEAR_RUNWAY", "PK101", 1);
    ch.logCommand("REROUTE", "PK202", 2);
    ch.logCommand("INTERCEPT", "PK303", 3);

    assert(ch.getHistoryCount() == 3);
    ch.displayHistory(3);
    ch.displayLastCommand();
    cout << "       -> Doubly Linked List tests passed!\n";
}

void testUndoRedoStacks() {
    cout << "[TEST] Running Undo and Redo Snapshot Stack tests...\n";
    CommandHistory ch;
    assert(!ch.canUndo());
    assert(!ch.canRedo());

    FlightSnapshot s1("PK101", "Runway 09", 9, "Air Segment", "ACTIVE", false, 1, "CLEAR_RUNWAY");
    FlightSnapshot s2("PK202", "Runway 04", 5, "Cache Fly", "ACTIVE", false, 2, "REROUTE");

    ch.pushUndoSnapshot(s1);
    ch.pushUndoSnapshot(s2);
    assert(ch.canUndo());

    ch.displayUndoStack();

    // Undo s2
    FlightSnapshot popped1 = ch.popUndoSnapshot();
    assert(popped1.callsign == "PK202");
    assert(popped1.prevSector == "Cache Fly");

    // Push into Redo
    ch.pushRedoSnapshot(popped1);
    assert(ch.canRedo());

    // Undo s1
    FlightSnapshot popped2 = ch.popUndoSnapshot();
    assert(popped2.callsign == "PK101");
    assert(!ch.canUndo());

    // Redo popped1
    FlightSnapshot redoPopped = ch.popRedoSnapshot();
    assert(redoPopped.callsign == "PK202");
    assert(!ch.canRedo());

    // Test clear redo
    ch.pushRedoSnapshot(s1);
    assert(ch.canRedo());
    ch.clearRedoStack();
    assert(!ch.canRedo());

    cout << "       -> Undo and Redo Stack tests passed!\n";
}

int main() {
    cout << "==========================================\n";
    cout << "       RUNNING STAGE 3 UNIT TESTS         \n";
    cout << "==========================================\n";
    testDoublyLinkedListHistory();
    testUndoRedoStacks();
    cout << "==========================================\n";
    cout << "   ALL STAGE 3 UNIT TESTS PASSED (100%)   \n";
    cout << "==========================================\n";
    return 0;
}
