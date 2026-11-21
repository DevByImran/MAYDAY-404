#ifndef EMERGENCY_LANDING_H
#define EMERGENCY_LANDING_H

#include <string>
#include <iostream>
#include "Flight.h"

using namespace std;


class NormalQueue {
private:
    Flight** queueArray;
    int capacity;
    int frontIdx;
    int rearIdx;
    int count;

    void resize();

public:
    explicit NormalQueue(int initialCap = 32);
    ~NormalQueue();

    NormalQueue(const NormalQueue&) = delete;
    NormalQueue& operator=(const NormalQueue&) = delete;

    void enqueue(Flight* f);
    Flight* dequeue();
    Flight* front() const;
    bool isEmpty() const { return count == 0; }
    int getCount() const { return count; }
    void display() const;
    bool removeFlight(const string& callsign);
    void clear();
};


class EmergencyMinHeap {
private:
    Flight** heapArray;
    int capacity;
    int size;

    void resize();
    void heapifyUp(int index);
    void heapifyDown(int index);
    bool isHigherPriority(Flight* a, Flight* b) const;

public:
    explicit EmergencyMinHeap(int initialCap = 32);
    ~EmergencyMinHeap();

    EmergencyMinHeap(const EmergencyMinHeap&) = delete;
    EmergencyMinHeap& operator=(const EmergencyMinHeap&) = delete;

    void insert(Flight* f);
    Flight* extractMin();
    Flight* peekMin() const;
    bool isEmpty() const { return size == 0; }
    int getSize() const { return size; }
    void display() const;
    bool removeFlight(const string& callsign);
    void clear();
};

class EmergencyLanding {
private:
    NormalQueue normalQueue;
    EmergencyMinHeap emergencyHeap;

public:
    EmergencyLanding();
    ~EmergencyLanding();

    
    void enqueueNormal(Flight* f);
    Flight* dequeueNormal();
    Flight* frontNormal() const;
    bool isEmptyNormal() const;
    void displayNormalQueue() const;


    void insertEmergency(Flight* f);
    Flight* extractMin();
    Flight* peekMin() const;
    bool isEmptyHeap() const;
    void displayHeap() const;


    Flight* dispatchNext();

    
    void removeFlightFromAll(const string& callsign);

    int getNormalCount() const { return normalQueue.getCount(); }
    int getEmergencyCount() const { return emergencyHeap.getSize(); }
    void displayAllQueues() const;
};


