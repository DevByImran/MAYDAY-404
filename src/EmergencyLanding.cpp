#include "EmergencyLanding.h"
#include <iomanip>

using namespace std;
//qUeue implementation

NormalQueue::NormalQueue(int initialCap)
    : capacity(initialCap), frontIdx(0), rearIdx(0), count(0) {
    queueArray = new Flight*[capacity];
    for (int i = 0; i < capacity; ++i) {
        queueArray[i] = nullptr;
    }
}

NormalQueue::~NormalQueue() {
    delete[] queueArray;
}

void NormalQueue::resize() {
    int newCap = capacity * 2;
    Flight** newArr = new Flight*[newCap];
    for (int i = 0; i < count; ++i) {
        newArr[i] = queueArray[(frontIdx + i) % capacity];
    }
    for (int i = count; i < newCap; ++i) {
        newArr[i] = nullptr;
    }
    delete[] queueArray;
    queueArray = newArr;
    frontIdx = 0;
    rearIdx = count;
    capacity = newCap;
}

void NormalQueue::enqueue(Flight* f) {
    if (f == nullptr) return;
    if (count >= capacity) {
        resize();
    }
    queueArray[rearIdx] = f;
    rearIdx = (rearIdx + 1) % capacity;
    count++;
}

Flight* NormalQueue::dequeue() {
    if (isEmpty()) return nullptr;
    Flight* f = queueArray[frontIdx];
    queueArray[frontIdx] = nullptr;
    frontIdx = (frontIdx + 1) % capacity;
    count--;
    return f;
}

Flight* NormalQueue::front() const {
    if (isEmpty()) return nullptr;
    return queueArray[frontIdx];
}

void NormalQueue::display() const {
    cout << "\n--------------------------------------------------\n";
    cout << "           NORMAL LANDING QUEUE (FIFO)            \n";
    cout << "--------------------------------------------------\n";
    if (isEmpty()) {
        cout << "Queue is empty.\n";
        return;
    }
    cout << left << setw(5) << "POS"
         << setw(10) << "CALLSIGN"
         << setw(12) << "TYPE"
         << setw(14) << "SECTOR"
         << setw(12) << "RUNWAY"
         << setw(8) << "ARR_TICK" << "\n";
    for (int i = 0; i < count; ++i) {
        Flight* f = queueArray[(frontIdx + i) % capacity];
        if (f != nullptr) {
            cout << left << setw(5) << (i + 1)
                 << setw(10) << f->callsign
                 << setw(12) << f->aircraftType
                 << setw(14) << f->currentSector
                 << setw(12) << f->assignedRunway
                 << setw(8) << f->arrivalTick << "\n";
        }
    }
    cout << "Total Waiting: " << count << "\n";
}

bool NormalQueue::removeFlight(const string& callsign) {
    if (isEmpty() || callsign.empty()) return false;

    int targetIdx = -1;
    for (int i = 0; i < count; ++i) {
        int idx = (frontIdx + i) % capacity;
        if (queueArray[idx] != nullptr && queueArray[idx]->callsign == callsign) {
            targetIdx = i;
            break;
        }
    }

    if (targetIdx == -1) return false;

    // Shift elements backwards
    for (int i = targetIdx; i < count - 1; ++i) {
        int curr = (frontIdx + i) % capacity;
        int next = (frontIdx + i + 1) % capacity;
        queueArray[curr] = queueArray[next];
    }
    rearIdx = (rearIdx - 1 + capacity) % capacity;
    queueArray[rearIdx] = nullptr;
    count--;
    return true;
}

void NormalQueue::clear() {
    for (int i = 0; i < capacity; ++i) {
        queueArray[i] = nullptr;
    }
    frontIdx = 0;
    rearIdx = 0;
    count = 0;
}

// min hEap implementation

EmergencyMinHeap::EmergencyMinHeap(int initialCap)
    : capacity(initialCap), size(0) {
    heapArray = new Flight*[capacity];
    for (int i = 0; i < capacity; ++i) {
        heapArray[i] = nullptr;
    }
}

EmergencyMinHeap::~EmergencyMinHeap() {
    delete[] heapArray;
}

void EmergencyMinHeap::resize() {
    int newCap = capacity * 2;
    Flight** newArr = new Flight*[newCap];
    for (int i = 0; i < size; ++i) {
        newArr[i] = heapArray[i];
    }
    for (int i = size; i < newCap; ++i) {
        newArr[i] = nullptr;
    }
    delete[] heapArray;
    heapArray = newArr;
    capacity = newCap;
}

bool EmergencyMinHeap::isHigherPriority(Flight* a, Flight* b) const {
    if (a == nullptr) return false;
    if (b == nullptr) return true;

    // 1. Lower priority numeric value has higher urgency
    if (a->priorityLevel != b->priorityLevel) {
        return a->priorityLevel < b->priorityLevel;
    }

    // 2. If equal priority, earlier arrival tick gets serviced first
    if (a->arrivalTick != b->arrivalTick) {
        return a->arrivalTick < b->arrivalTick;
    }

    // Secondary tie-breaker by remaining fuel (lower fuel first)
    return a->fuelRemaining < b->fuelRemaining;
}

void EmergencyMinHeap::heapifyUp(int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (isHigherPriority(heapArray[index], heapArray[parent])) {
            Flight* temp = heapArray[index];
            heapArray[index] = heapArray[parent];
            heapArray[parent] = temp;
            index = parent;
        } else {
            break;
        }
    }
}

void EmergencyMinHeap::heapifyDown(int index) {
    while (true) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int smallest = index;

        if (left < size && isHigherPriority(heapArray[left], heapArray[smallest])) {
            smallest = left;
        }
        if (right < size && isHigherPriority(heapArray[right], heapArray[smallest])) {
            smallest = right;
        }

        if (smallest != index) {
            Flight* temp = heapArray[index];
            heapArray[index] = heapArray[smallest];
            heapArray[smallest] = temp;
            index = smallest;
        } else {
            break;
        }
    }
}

void EmergencyMinHeap::insert(Flight* f) {
    if (f == nullptr) return;
    if (size >= capacity) {
        resize();
    }
    heapArray[size] = f;
    heapifyUp(size);
    size++;
}

Flight* EmergencyMinHeap::extractMin() {
    if (isEmpty()) return nullptr;
    Flight* minFlight = heapArray[0];
    heapArray[0] = heapArray[size - 1];
    heapArray[size - 1] = nullptr;
    size--;
    if (size > 0) {
        heapifyDown(0);
    }
    return minFlight;
}

Flight* EmergencyMinHeap::peekMin() const {
    if (isEmpty()) return nullptr;
    return heapArray[0];
}

void EmergencyMinHeap::display() const {
    cout << "\n--------------------------------------------------\n";
    cout << "           EMERGENCY MIN HEAP (PRIORITY)          \n";
    cout << "--------------------------------------------------\n";
    if (isEmpty()) {
        cout << "No pending emergency flights.\n";
        return;
    }
    cout << left << setw(5) << "HEAP"
         << setw(10) << "CALLSIGN"
         << setw(14) << "PRIORITY"
         << setw(12) << "LEVEL_NUM"
         << setw(10) << "ARR_TICK"
         << setw(8) << "FUEL" << "\n";

    for (int i = 0; i < size; ++i) {
        Flight* f = heapArray[i];
        if (f != nullptr) {
            cout << left << setw(5) << (i + 1)
                 << setw(10) << f->callsign
                 << setw(14) << Flight::getPriorityName(f->priorityLevel)
                 << setw(12) << f->priorityLevel
                 << setw(10) << f->arrivalTick
                 << setw(8) << f->fuelRemaining << "\n";
        }
    }
    cout << "Total Emergency Flights: " << size << "\n";
}

bool EmergencyMinHeap::removeFlight(const string& callsign) {
    if (isEmpty() || callsign.empty()) return false;

    int targetIdx = -1;
    for (int i = 0; i < size; ++i) {
        if (heapArray[i] != nullptr && heapArray[i]->callsign == callsign) {
            targetIdx = i;
            break;
        }
    }

    if (targetIdx == -1) return false;

    heapArray[targetIdx] = heapArray[size - 1];
    heapArray[size - 1] = nullptr;
    size--;

    if (targetIdx < size) {
        heapifyUp(targetIdx);
        heapifyDown(targetIdx);
    }
    return true;
}

void EmergencyMinHeap::clear() {
    for (int i = 0; i < capacity; ++i) {
        heapArray[i] = nullptr;
    }
    size = 0;
}

// Emergencylanding implementation

EmergencyLanding::EmergencyLanding() {}
EmergencyLanding::~EmergencyLanding() {}

void EmergencyLanding::enqueueNormal(Flight* f) {
    normalQueue.enqueue(f);
}

Flight* EmergencyLanding::dequeueNormal() {
    return normalQueue.dequeue();
}

Flight* EmergencyLanding::frontNormal() const {
    return normalQueue.front();
}

bool EmergencyLanding::isEmptyNormal() const {
    return normalQueue.isEmpty();
}

void EmergencyLanding::displayNormalQueue() const {
    normalQueue.display();
}

void EmergencyLanding::insertEmergency(Flight* f) {
    emergencyHeap.insert(f);
}

Flight* EmergencyLanding::extractMin() {
    return emergencyHeap.extractMin();
}

Flight* EmergencyLanding::peekMin() const {
    return emergencyHeap.peekMin();
}

bool EmergencyLanding::isEmptyHeap() const {
    return emergencyHeap.isEmpty();
}

void EmergencyLanding::displayHeap() const {
    emergencyHeap.display();
}

Flight* EmergencyLanding::dispatchNext() {
    // Assignment Dispatch Rule:
    // If the normal landing queue is not empty, process its front flight.
    // Otherwise, extract the highest-priority emergency from the heap.
    if (!normalQueue.isEmpty()) {
        return normalQueue.dequeue();
    }
    if (!emergencyHeap.isEmpty()) {
        return emergencyHeap.extractMin();
    }
    return nullptr;
}

void EmergencyLanding::removeFlightFromAll(const string& callsign) {
    normalQueue.removeFlight(callsign);
    emergencyHeap.removeFlight(callsign);
}

void EmergencyLanding::displayAllQueues() const {
    normalQueue.display();
    emergencyHeap.display();
}
