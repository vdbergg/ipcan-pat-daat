//
// Created by berg on 13/01/19.
//

#include "../header/Trie.h"
#include "../header/Experiment.h"
#include "../header/Directives.h"
#include <map>

vector<StaticString> records;

Trie::Trie(Experiment* experiment) {
    this->experiment = experiment;
//    this->root = newNode();
//    newNode(); // Makes nobody become node 0, so 0 is NULL
    this->root = newNode(); // root is node 1
    getNode(this->root).setBeginRange(0);
    getNode(this->root).setParent(0);
//    getNode(this->root).setLevel(-1); // root has level -1
    getNode(this->root).setEndRange(records.size());
    getNode(this->root).setIsLeaf(true);
    this->experiment->incrementNumberOfNodes();

    this->globalMemory.reserve(records.size() * 3);

    this->lastNodeKnownPerRecord.reserve(records.size());
    this->nextIthCharKnownPerRecord.reserve(records.size());

    for (int recordId = 0; recordId < records.size(); recordId++) {
        this->lastNodeKnownPerRecord[recordId] = this->nextIthCharKnownPerRecord[recordId] = this->root;
    }
}

void Trie::buildDaatIndex() {
    int maxDepth = records[0].length();
    for (int recordId = 1; recordId < records.size(); recordId++){
        maxDepth = max((int) string(records[recordId].c_str()).size(), maxDepth);
        #ifdef IS_COLLECT_TIME_H
        this->experiment->proportionOfBranchingSize((int) string(records[recordId].c_str()).size());
        #endif
    }

    for (int currentDepth = 0; currentDepth < maxDepth; currentDepth++) {
        unordered_map<unsigned, unsigned> numberOfChildrenCreated;

        unsigned oldParent = this->lastNodeKnownPerRecord[0];
        for (int recordId = 0; recordId < records.size(); recordId++) {

            int ithCharFromRecord = nextIthCharKnownPerRecord[recordId];
            if (ithCharFromRecord < records[recordId].length()) {
                unsigned parent = this->lastNodeKnownPerRecord[recordId];

                if (oldParent != parent) {
                    oldParent = parent;
                    this->mergeSinglesChildWithParent(numberOfChildrenCreated);
                }

                unsigned node = this->insertDaat(ithCharFromRecord, recordId, parent, numberOfChildrenCreated);

                this->lastNodeKnownPerRecord[recordId] = node;
                this->nextIthCharKnownPerRecord[recordId] = ithCharFromRecord;
            }
        }
        this->mergeSinglesChildWithParent(numberOfChildrenCreated);
    }
}

void Trie::mergeSinglesChildWithParent(unordered_map<unsigned, unsigned>& numberOfChildrenCreated) {
    unordered_map<unsigned, unsigned>:: iterator itr;
    for (itr = numberOfChildrenCreated.begin(); itr != numberOfChildrenCreated.end(); itr++) {
        unsigned parent = itr->first;
        unsigned numberOfChildren = itr->second;

        if (numberOfChildren == 1) {
            unsigned child = getNode(parent).children + getNode(parent).numChildren - 1;
            for (unsigned recordId = getNode(child).getBeginRange(); recordId < getNode(child).getEndRange(); recordId++) {
                this->lastNodeKnownPerRecord[recordId] = parent;
            }

            this->removeNode();
            getNode(parent).setIsLeaf(true);
            getNode(parent).numChildren--;
            getNode(parent).label.stringSize++;

            // Case when a string contains another string
            unsigned recordId = getNode(parent).getBeginRange();
            bool isEndOfWord = this->nextIthCharKnownPerRecord[recordId] - 1 == records[recordId].length() - 1;
            getNode(parent).setIsEndOfWord(isEndOfWord);

            #ifdef IS_COLLECT_TIME_H
                this->experiment->decrementNumberOfNodes();
            #endif
        }
    }

    numberOfChildrenCreated.clear();
}

unsigned Trie::insertDaat(int &ithCharFromRecord, int recordId, unsigned parent,
        unordered_map<unsigned, unsigned>& numberOfChildrenCreated) {
    unsigned numChildren = getNode(parent).numChildren;

    if (numChildren) {
        unsigned child = getNode(parent).children + numChildren - 1;
        if (getNode(child).getValue() == records[recordId][ithCharFromRecord]) {
            getNode(child).setEndRange(recordId + 1);
            ithCharFromRecord++;
            return child;
        }
    }

    unsigned node = newNode(records[recordId].staticSubstr(ithCharFromRecord, 1));
    if (numChildren == 0) getNode(parent).children = node;
    getNode(parent).numChildren++;
    getNode(parent).setIsLeaf(false);
    getNode(node).setBeginRange(recordId);
    getNode(node).setEndRange(recordId + 1);
    getNode(node).setIsLeaf(true);
    getNode(node).setParent(parent);
//    getNode(node).setLevel(getNode(parent).level + 1);
    // Case when a string contains another string
    getNode(node).setIsEndOfWord(ithCharFromRecord == records[recordId].length() - 1);

    #ifdef IS_COLLECT_TIME_H
        this->experiment->incrementNumberOfNodes();
    #endif

    ithCharFromRecord++;

    // Case when a string contains another string
    if (getNode(parent).getIsEndOfWord()) return node;

    if (numberOfChildrenCreated.find(parent) == numberOfChildrenCreated.end()) {
        numberOfChildrenCreated[parent] = 1;
    } else {
        numberOfChildrenCreated[parent] += 1;
    }

    return node;
}

void Trie::shrinkToFit() {
    this->globalMemory.shrink_to_fit();

    this->lastNodeKnownPerRecord.clear();
    this->lastNodeKnownPerRecord.shrink_to_fit();
    this->nextIthCharKnownPerRecord.clear();
    this->nextIthCharKnownPerRecord.shrink_to_fit();
}
