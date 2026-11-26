#ifndef AVL_FLIGHT_H
#define AVL_FLIGHT_H

#include <string>
#include <iostream>
#include "Flight.h"

using namespace std;

struct AVLNode {
    string key;
    Flight* flight;
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(Flight* f)
        : key(f ? f->callsign : ""), flight(f), left(nullptr), right(nullptr), height(1) {}
};

class AVLflight {
private:
    AVLNode* root;
    int nodeCount;

    // Helper functions
    int getNodeHeight(AVLNode* n) const;
    int getBalanceFactor(AVLNode* n) const;
    void updateHeight(AVLNode* n);

    // Rotations
    AVLNode* rotateRight(AVLNode* y);     // LL Case
    AVLNode* rotateLeft(AVLNode* x);      // RR Case
    AVLNode* rotateLeftRight(AVLNode* n); // LR Case
    AVLNode* rotateRightLeft(AVLNode* n); // RL Case

    AVLNode* insertHelper(AVLNode* node, Flight* f, bool& inserted);
    AVLNode* searchHelper(AVLNode* node, const string& callsign) const;
    void inorderHelper(AVLNode* node) const;
    AVLNode* findMin(AVLNode* node) const;
    AVLNode* eraseHelper(AVLNode* node, const string& callsign, bool& deleted);
    void destroyHelper(AVLNode* node);

public:
    AVLflight();
    ~AVLflight();

    AVLflight(const AVLflight&) = delete;
    AVLflight& operator=(const AVLflight&) = delete;

    bool insert(Flight* f);
    Flight* search(const string& callsign) const;
    void inorderTraversal() const;
    int height() const;
    bool erase(const string& callsign);
    void destroyTree();
    int getNodeCount() const { return nodeCount; }
    bool isEmpty() const { return root == nullptr; }

    // Demonstration comparison helper
    static void runBalancingDemonstration();
};

#endif // AVL_FLIGHT_H
