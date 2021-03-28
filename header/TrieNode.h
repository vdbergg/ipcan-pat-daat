//
// Created by berg on 30/03/2020.
//

#ifndef BEVA_NODE_H
#define BEVA_NODE_H

#include <unordered_map>
#include <iostream>
#include <vector>
#include "ShortVector.h"
#include <vector>
#include "StaticString.h"

#define CHAR_SIZE 128

using namespace std;


class TrieNode {
 public:
//  ShortVector<unsigned long > children;
    unsigned long children;
    unsigned long parent;

    unsigned beginRange = -1;
    unsigned endRange;
//    short level; // Root level should start with -1;
    unsigned char numChildren;
    bool isLeaf;
    bool isEndOfWord;
    StaticString label;

    TrieNode() {
        this->numChildren = 0;
        this->setIsLeaf(false);
//        this->children.clear();
    }

    TrieNode(StaticString str) {
        this->label = str;
        this->numChildren = 0;
        this->setIsLeaf(false);
//        this->children.clear();
    }
 
    unsigned getChildrenCount() {
        return this->numChildren;
    }

    unsigned long getChild(unsigned childIterator) {
        return this->children + childIterator;
    }

    inline char getValue() { return this->label[0]; }
    inline void setIsLeaf(bool value) { this->isLeaf = value; }
    inline bool getIsLeaf() { return this->isLeaf; }
    inline void setIsEndOfWord(bool value) { this->isEndOfWord = value; }
    inline bool getIsEndOfWord() { return this->isEndOfWord; }

    inline unsigned long getParent() { return this->parent; }
    inline void setParent(unsigned long value) { this->parent = value; }
//    inline void setLevel(unsigned short value) { this->level = value; }
    inline char getCharacter(unsigned ithCharFromLabel) { return this->label[ithCharFromLabel]; }
    inline bool isTerminalNode() { return this->label[0] == 0; }
//    inline unsigned short getLevel() { return this->level; }
//    inline unsigned short getDepth() { return this->level; }
    inline unsigned getBeginRange() { return this->beginRange; }
    inline unsigned getEndRange() { return this->endRange;}
    inline void setBeginRange(unsigned value) { this->beginRange = value; }
    inline void setEndRange(unsigned value) { this->endRange = value; }
};

#endif //BEVA_NODE_H
