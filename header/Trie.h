//
// Created by berg on 13/01/19.
//

#include <string>
#include "TrieNode.h"
#include "Experiment.h"
#include "StaticString.h"

#ifndef BEVA_TRIE_H
#define BEVA_TRIE_H

using namespace std;

extern vector<StaticString> records;

class Trie {
public:
    vector<TrieNode> globalMemory;
    unsigned long root;
    vector<unsigned> lastNodeKnownPerRecord;
    vector<unsigned> nextIthCharKnownPerRecord;
    Experiment* experiment;

    Trie(Experiment*);

    void buildDaatIndex();
    unsigned insertDaat(int &ithCharFromRecord, int recordId, unsigned parent,
            unordered_map<unsigned, unsigned>& numberOfChildrenCreated);

    void mergeSinglesChildWithParent(unordered_map<unsigned, unsigned>& numberOfChildrenCreated);

    // used by Ipcan
    inline unsigned long getRoot() {
        return this->root;
    }

    inline TrieNode* getNodeAddr(unsigned x) {
        return &this->globalMemory[x];
    }

    inline void removeNode() {
        this->globalMemory.resize(this->globalMemory.size() - 1);
    }

    inline TrieNode& getNode(unsigned x) {
        if (x >= this->globalMemory.size()) {
            cout << "ERRO\n";
            exit(1);
        }
        return this->globalMemory[x];
    }

    inline unsigned newNode() {
        this->globalMemory.emplace_back();
        return this->globalMemory.size() - 1;
    }

    inline unsigned newNode(const StaticString& str) {
        this->globalMemory.emplace_back(str);
        return this->globalMemory.size() - 1;
    }

    void shrinkToFit();
};

#endif //BEVA_TRIE_H
