#ifndef BST_FLIGHT_H
#define BST_FLIGHT_H

#include <string>
#include <iostream>
#include "Flight.h"

using namespace std;

struct BSTNode {
    string key;
    Flight* flight;
    BSTNode* left;
    BSTNode* right;

    BSTNode(Flight* f)
        : key(f ? f->callsign : ""), flight(f), left(nullptr), right(nullptr) {}
};

class BSTflight {
private:
    BSTNode* root;
    int nodeCount;

    BSTNode* insertHelper(BSTNode* node, Flight* f, bool& inserted);
    BSTNode* searchHelper(BSTNode* node, const string& callsign) const;
    void inorderHelper(BSTNode* node) const;
    int heightHelper(BSTNode* node) const;
    BSTNode* findMin(BSTNode* node) const;
    BSTNode* eraseHelper(BSTNode* node, const string& callsign, bool& deleted);
    void destroyHelper(BSTNode* node);

public:
    BSTflight();
    ~BSTflight();

    BSTflight(const BSTflight&) = delete;
    BSTflight& operator=(const BSTflight&) = delete;

    bool insert(Flight* f);
    Flight* search(const string& callsign) const;
    void inorderTraversal() const;
    int height() const;
    bool erase(const string& callsign);
    void destroyTree();
    int getNodeCount() const { return nodeCount; }
    bool isEmpty() const { return root == nullptr; }
};

#endif // BST_FLIGHT_H
