
// $Id: ActiveNode.h 638 2010-11-28 14:49:05Z liguoliang $

#ifndef __BEVA_ACTIVENODEIPCAN_H__
#define __BEVA_ACTIVENODEIPCAN_H__

#include "Trie.h"

#include <vector>
#include <map>
#include <set>
#include <iostream>

/*
PAN:
*/
typedef struct _PivotalActiveNode
{
    unsigned transformationdistance;
    unsigned short differ; // |p_{x+1}-p_i|
    unsigned short editdistanceofPrefix;
    unsigned short ithCharFromLabel;
} PivotalActiveNode;

struct CustomTrieNode {
    unsigned long trieNode;
    unsigned ithCharFromLabel;

    CustomTrieNode(unsigned long trieNode, unsigned ithCharFromLabel)
    {
        this->trieNode = trieNode;
        this->ithCharFromLabel = ithCharFromLabel;
    }

    bool operator==(const CustomTrieNode& customTrieNode) const
    {
        return this->trieNode == customTrieNode.trieNode && this->ithCharFromLabel == customTrieNode.ithCharFromLabel;
    }

    bool operator<(const CustomTrieNode &customTrieNode)  const
    {
        return this->trieNode < customTrieNode.trieNode || (this->trieNode == customTrieNode.trieNode && this->ithCharFromLabel < customTrieNode.ithCharFromLabel);
    }
};

//class MyHashFunction {
//public:
//
//    size_t operator()(const CustomTrieNode& customTrieNode) const
//    {
//        return customTrieNode.trieNode + customTrieNode.ithCharFromLabel;
//    }
//};

/*
typedef TrieNode<charT> TrieNode_Internal;
typedef Trie<charT> Trie_Internal;
 */

template <class charT>
struct ResultNode
{
    const unsigned long node;
    int editDistance;
    int prefixLength;
    ResultNode(unsigned long in_node) : node(in_node) {}
    ResultNode(const unsigned long in_node, int in_editDistance, int in_prefixLength) : node(in_node),
                                                                                           editDistance(in_editDistance), prefixLength(in_prefixLength) {}
};

template <class charT>
class PrefixActiveNodeSet
{
public:
    typedef std::vector<CustomTrieNode> TrieNodeSet;
    Trie *trie;

    void init(std::string &prefix, const unsigned editDistanceThreshold)
    {
        this->prefix = prefix;
        this->editDistanceThreshold = editDistanceThreshold;

        this->trieNodeSetVector.clear();
        this->trieNodeSetVectorComputed = false;

        this->flagResultsCached = false;
    }

    PrefixActiveNodeSet(std::string &prefix, const unsigned editDistanceThreshold)
    {
        init(prefix, editDistanceThreshold);
    }

    /// A set of active nodes for an empty string and an edit-distance threshold
    PrefixActiveNodeSet(Trie* trie, const unsigned editDistanceThreshold)
    {
        std::string emptyString = "";
        init(emptyString, editDistanceThreshold);

        //PAN:
        // Add the trie nodes up to the given depth
        unsigned long root = trie->getRoot();
        CustomTrieNode customTrieNode(root, 0);
        _addActiveNode(customTrieNode, 0); // add the root first
        PivotalActiveNode pan;
        pan.transformationdistance = 0;
        pan.differ = 0; // |p_{x+1}-p_i|
        pan.editdistanceofPrefix = 0;
        pan.ithCharFromLabel = 0;
        _addPAN(customTrieNode, pan);
        //if (editDistanceThreshold > 0)
        //	addTrieNodesUpToDepth(root, editDistanceThreshold, 0);
    }

    ~PrefixActiveNodeSet()
    {
    }

    PrefixActiveNodeSet<charT> *computeActiveNodeSetIncrementally(const char additionalChar, Trie *trie)
    {
        this->trie = trie;
        // form the new string. // TODO (OPT): avoid string copy
        std::string newString = this->prefix;
        newString.push_back(additionalChar);

        PrefixActiveNodeSet<charT> *newActiveNodeSet = new PrefixActiveNodeSet<charT>(newString, this->getEditDistanceThreshold());

        // PAN:
        for (typename std::map<CustomTrieNode, PivotalActiveNode>::iterator mapIterator = PANMap.begin();
             mapIterator != PANMap.end(); mapIterator++)
        {
            // Compute the new active nodes for this trie node
            _addPANSetForOneNode(mapIterator->first, mapIterator->second, additionalChar, newActiveNodeSet);
        }
        /* iterate over all the active nodes
        for (typename std::map<TrieNode<charT>*, unsigned >::iterator mapIterator = trieNodeDistanceMap.begin();
             mapIterator != trieNodeDistanceMap.end(); mapIterator ++) {
            // Compute the new active nodes for this trie node
            _addActiveNodeSetForOneNode(mapIterator->first, mapIterator->second, additionalChar, newActiveNodeSet);
        }*/

        //PAN: update active node
        for (typename std::map<CustomTrieNode, PivotalActiveNode>::iterator mIterator = newActiveNodeSet->PANMap.begin();
             mIterator != newActiveNodeSet->PANMap.end(); mIterator++)
        {
            newActiveNodeSet->_addActiveNode(mIterator->first, mIterator->second.transformationdistance);
        }
        return newActiveNodeSet;
    }

    bool fillResultNodeSet(std::vector<ResultNode<charT>> &result, int size = -1) const
    {
        return 0;
    }

    unsigned getEditDistanceThreshold() const
    {
        return editDistanceThreshold;
    }

    unsigned getNumberOfBytes() const
    {

        unsigned trieNodeSetVectorSize = 0;

        for (typename std::vector<TrieNodeSet>::const_iterator vectorIter = trieNodeSetVector.begin(); vectorIter != trieNodeSetVector.end(); vectorIter++)
        {
            trieNodeSetVectorSize += (*vectorIter).capacity() * sizeof(*vectorIter);
        }
        trieNodeSetVectorSize += trieNodeSetVector.size() * sizeof(TrieNodeSet) + (this->trieNodeSetVector.capacity() - trieNodeSetVector.size()) * sizeof(void *);

        return this->prefix.size() + sizeof(this->editDistanceThreshold) + trieNodeDistanceMap.size() * (sizeof(void *) + sizeof(unsigned)) + sizeof(trieNodeDistanceMap) + trieNodeSetVectorSize;
    }

    unsigned getNumberOfActiveNodes()
    {
        return (unsigned)trieNodeDistanceMap.size();
    }

    std::string *getPrefix()
    {
        return &prefix;
    }

    unsigned getPrefixLength() const
    {
        return prefix.length();
    }

    void getComputedSimilarPrefixes(std::vector<std::string> &similarPrefixes)
    {
//        for (typename std::map<unsigned long, unsigned>::iterator mapIterator = trieNodeDistanceMap.begin();
//             mapIterator != trieNodeDistanceMap.end(); mapIterator++)
//        {
//            unsigned long trieNode = mapIterator->first;
//            std::string prefix;
//            trieNode->getPrefix(prefix);
//            similarPrefixes.push_back(prefix);
//        }
    }

    //typedef std::vector<TrieNode<charT>* > TrieNodeSet;
    std::vector<TrieNodeSet> *getTrieNodeSetVector()
    {

        // compute it only if necessary
        if (this->trieNodeSetVectorComputed)
            return &trieNodeSetVector;

        _computeTrieNodeSetVector();
        return &trieNodeSetVector;
    }

    // add the descendants of the current node up to "depth" to the trieNodeDistanceMap with
    // the corresponding edit distance.  The edit distance of the current node is "editDistance".
    void addTrieNodesUpToDepth(unsigned long trieNode, const unsigned depthLimit, const unsigned editDistance)
    {
//        unsigned currentDepth = this->trie->getNode(trieNode).getDepth();
//        // ASSERT(currentDdepth < depthLimit); // this node should have been added by its parent
//
//        // add children
//        unsigned int childrenCount = this->trie->getNode(trieNode).getChildrenCount();
//        for (unsigned int childIterator = 0; childIterator < childrenCount; childIterator++)
//        {
//            unsigned long child = this->trie->getNode(trieNode).getChild(childIterator);
//            _addActiveNode(child, editDistance + 1); // add one more node
//            if (currentDepth + 1 < depthLimit)		 // recursive call for each child
//                addTrieNodesUpToDepth(child, depthLimit, editDistance + 1);
//        }
    }

    // The following function is used to implement the iterator to go through the leaf nodes.

    // Compute the set of active nodes with an edit distance to the prefix within a given range.
    // Minimize them by not including active nodes whose ancestor is already included.
    // The goal is to make it more efficient to compute their leaf nodes
    void computeMinimizedTrieNodesInRange(unsigned minEd, unsigned maxEd,
            std::map<CustomTrieNode, unsigned> &minimizedTrieNodesMap, Trie *trie)
    {
        this->trie = trie;
        // ASSERT(minEd <= maxEd);
        // ASSERT(maxEd <= editDistanceThreshold);

        // TODO (OPT): avoid generating a temporary map
        std::map<CustomTrieNode, unsigned> trieNodes, *trieNodesPointer;

        // no need to compute the temporary map if the range is [0, edThreshold]
        if (minEd == 0 && maxEd == editDistanceThreshold)
        {
            trieNodesPointer = &(this->trieNodeDistanceMap);
        }
        else
        { // we need to compute it
            for (typename std::map<CustomTrieNode, unsigned>::const_iterator mapIterator = this->trieNodeDistanceMap.begin();
                 mapIterator != this->trieNodeDistanceMap.end(); mapIterator++)
            {
                if (mapIterator->second >= minEd && mapIterator->second <= maxEd)
                    trieNodes[mapIterator->first] = mapIterator->second; // add it to the new map
            }
            trieNodesPointer = &(trieNodes); // pointing to the new one
        }

        // compute the minimizedTrieNodesMap
        minimizedTrieNodesMap.clear();
        for (typename std::map<CustomTrieNode, unsigned>::const_iterator mapIterator = trieNodesPointer->begin();
             mapIterator != trieNodesPointer->end(); mapIterator++)
        {
            // add it to the map only if it doesn't have an ancestor in the set
            if (_hasAncestorInMap(trieNodesPointer, mapIterator->first) == false)
                minimizedTrieNodesMap[mapIterator->first] = mapIterator->second;
        }
    }

    bool isResultsCached() const
    {
        return this->flagResultsCached;
    }

    void setResultsCached(bool flag)
    {
        this->flagResultsCached = flag;
    }

    std::map<CustomTrieNode, PivotalActiveNode> PANMap;

private:
    std::string prefix;
    unsigned editDistanceThreshold;
    bool flagResultsCached;

    // A map from trie node to its edit distance to the prefix
    std::map<CustomTrieNode, unsigned> trieNodeDistanceMap;

    //PAN: A map from trie node to its pivotal active nodes

    // group the trie nodes based on their edit distance to the prefix.
    // used only when it's called by an interator
    std::vector<TrieNodeSet> trieNodeSetVector;
    bool trieNodeSetVectorComputed; // indicated if the trieNodeSetVector has been computed

    //PAN:

    /// compute the pivotal active nodes based on one of the active nodes of the previous prefix
    /// add the new pivotal active nodes to newActiveNodeSet
    void _addPANSetForOneNode(CustomTrieNode customTrieNode, PivotalActiveNode pan,
                              const char additionalChar, PrefixActiveNodeSet<charT> *newActiveNodeSet)
    {
        // deletion
        PivotalActiveNode dpan;
        dpan.transformationdistance = pan.transformationdistance + 1;
        dpan.differ = pan.differ + 1;
        dpan.editdistanceofPrefix = pan.editdistanceofPrefix;
        dpan.ithCharFromLabel = pan.ithCharFromLabel;
        newActiveNodeSet->_addPAN(customTrieNode, dpan);

        // go through the children of this treNode
        int depthLimit = this->getEditDistanceThreshold() - pan.editdistanceofPrefix + 1;
        int curDepth = 0;
        addPANUpToDepth(customTrieNode, pan, curDepth, depthLimit, additionalChar, newActiveNodeSet);
    }

    //PAN:
    /// Add a new pivotal active node with an edit distance.
    /// If the pivotal active node already exists in the set and had a distance no greater than the new one,
    /// then ignore this request.
    void _addPAN(CustomTrieNode customTrieNode, PivotalActiveNode pan)
    {
        if (pan.transformationdistance > this->editDistanceThreshold) // do nothing if the new distance is above the threshold
            return;
        //PAN:
        typename std::map<CustomTrieNode, PivotalActiveNode>::iterator mapIterator = PANMap.find(customTrieNode);
        if (mapIterator != PANMap.end())
        {																				 // found one
            if (mapIterator->second.transformationdistance > pan.transformationdistance) // reassign the distance if it's smaller
                mapIterator->second = pan;
            else if (mapIterator->second.transformationdistance == pan.transformationdistance)
            {
                if ((mapIterator->second.differ < pan.differ) || (mapIterator->second.editdistanceofPrefix > pan.editdistanceofPrefix))
                    mapIterator->second = pan;
            }
            return; // otherwise, do nothing
        }

        // insert the new pair
        PANMap.insert(std::pair<CustomTrieNode, PivotalActiveNode>(customTrieNode, pan));

        // set the flag
        this->trieNodeSetVectorComputed = false;
    }

    void addChar(CustomTrieNode customTrieNode, PivotalActiveNode pan, const unsigned curDepth,
            const unsigned depthLimit, const char additionalChar, PrefixActiveNodeSet<charT> *newActiveNodeSet,
            char ch, unsigned nextCharFromLabel, int max) {
        pan.ithCharFromLabel = nextCharFromLabel;

        if (ch == additionalChar) { // match
            PivotalActiveNode panlocal;
            panlocal.transformationdistance = pan.editdistanceofPrefix + max;
            panlocal.differ = 0;
            panlocal.editdistanceofPrefix = pan.editdistanceofPrefix + max;
            panlocal.ithCharFromLabel = pan.ithCharFromLabel;
            newActiveNodeSet->_addPAN(customTrieNode, panlocal);
        }

        if (curDepth + 1 < depthLimit) { // recursive call for each child
            addPANUpToDepth(customTrieNode, pan, curDepth + 1, depthLimit, additionalChar, newActiveNodeSet);
        }
    }

    //PAN:
    // add the descendants of the current node up to "depth" to the PANMao with
    // the corresponding edit distance.  The edit distance of the current node is "editDistance".
    void addPANUpToDepth(CustomTrieNode customTrieNode, PivotalActiveNode pan, const unsigned curDepth,
            const unsigned depthLimit, const char additionalChar, PrefixActiveNodeSet<charT> *newActiveNodeSet) {
        // add children
        int max = curDepth;
        if (max < pan.differ)
            max = pan.differ;

        auto &curNode = trie->getNode(customTrieNode.trieNode);
        if (curNode.label.size() == pan.ithCharFromLabel) {
            unsigned int childrenCount = this->trie->getNode(customTrieNode.trieNode).getChildrenCount();

            for (unsigned int childIterator = 0; childIterator < childrenCount; childIterator++) {
                unsigned long child = this->trie->getNode(customTrieNode.trieNode).getChild(childIterator);
                CustomTrieNode c(child, 0);
                addChar(c, pan, curDepth, depthLimit, additionalChar, newActiveNodeSet,
                        this->trie->getNode(child).getValue(), 1, max);
            }
        } else {
            CustomTrieNode c(customTrieNode.trieNode, pan.ithCharFromLabel);
            addChar(c, pan, curDepth, depthLimit, additionalChar, newActiveNodeSet,
                    this->trie->getNode(customTrieNode.trieNode).getCharacter(pan.ithCharFromLabel), pan.ithCharFromLabel + 1, max);
        }
    }

    /// compute the active nodes based on one of the active nodes of the previous prefix
    /// add the new active nodes to newActiveNodeSet
    void _addActiveNodeSetForOneNode(unsigned long trieNode, unsigned distance,
                                     const char additionalChar, PrefixActiveNodeSet<charT> *newActiveNodeSet)
    {
        // deletion (from the new string's perspective
//        newActiveNodeSet->_addActiveNode(trieNode, distance + 1);
//
//        // go through the children of this treNode
//        unsigned int childrenCount = this->trie->getNode(trieNode).getChildrenCount();
//        for (unsigned int childIterator = 0; childIterator < childrenCount; childIterator++)
//        {
//            unsigned long child = this->trie->getNode(trieNode).getChild(childIterator);
//            if (this->trie->getNode(child).getCharacter() != additionalChar)
//            { // substitution
//                newActiveNodeSet->_addActiveNode(child, distance + 1);
//            }
//            else
//            { // the two chars match
//                newActiveNodeSet->_addActiveNode(child, distance);
//
//                // Insert the descendants of this child up to a certain depth
//                if (distance < editDistanceThreshold)
//                { // do the math here
//                    unsigned additionalSteps = editDistanceThreshold - distance;
//                    newActiveNodeSet->addTrieNodesUpToDepth(child, this->trie->getNode(child).getDepth() + additionalSteps, distance);
//                }
//            }
//        }
    }

    //PAN:
    /// Add a new active node with an edit distance.
    /// If the active node already exists in the set and had a distance no greater than the new one,
    /// then ignore this request.
    void _addActiveNode(CustomTrieNode customTrieNode, unsigned editDistance)
    {
        if (editDistance > this->editDistanceThreshold) // do nothing if the new distance is above the threshold
            return;

        //PAN:
        typename std::map<CustomTrieNode, unsigned>::iterator mapIterator = trieNodeDistanceMap.find(customTrieNode);
        if (mapIterator != trieNodeDistanceMap.end())
        {											// found one
            if (mapIterator->second > editDistance) // reassign the distance if it's smaller
                mapIterator->second = editDistance;
            return; // otherwise, do nothing
        }

        // insert the new pair
        trieNodeDistanceMap.insert(std::pair<CustomTrieNode, unsigned>(customTrieNode, editDistance));

        // set the flag
        this->trieNodeSetVectorComputed = false;
    }

    void _computeTrieNodeSetVector()
    {
        if (this->trieNodeSetVectorComputed)
            return;

        // VECTOR: initialize the vector
        this->trieNodeSetVector.resize(editDistanceThreshold + 1);
        for (unsigned i = 0; i <= editDistanceThreshold; i++)
            this->trieNodeSetVector[i].clear();

        // go over the map to populate the vectors.
        for (typename std::map<CustomTrieNode, unsigned>::iterator mapIterator = trieNodeDistanceMap.begin();
             mapIterator != trieNodeDistanceMap.end(); mapIterator++)
        {
            this->trieNodeSetVector[mapIterator->second].push_back(mapIterator->first);
        }

        // set the flag
        this->trieNodeSetVectorComputed = true;
    }

    // check if the given trieNode has an ancestor (not including itself) in the given set of active nodes
    bool _hasAncestorInMap(std::map<CustomTrieNode, unsigned> *trieNodeMap, CustomTrieNode customTrieNode)
    {

        for (unsigned long node = this->trie->getNode(customTrieNode.trieNode).getParent(); node != 0; node = this->trie->getNode(node).getParent())
        {
            CustomTrieNode c(node, customTrieNode.ithCharFromLabel);
            if (trieNodeMap->find(c) != trieNodeMap->end()) // found one
                return true;
        }
        return false;
    }
};

/*
 * An iterator to go through the active nodes and their distances stored in a PrefixActiveNodeSet.
 * Design based on the example at http://sourcemaking.com/design_patterns/iterator/cpp/1
 *
 * An example:
 * PrefixActiveNodeSet<charT> *prefixActiveNodeSet;
       for (ActiveNodeSetIterator<charT> ani(prefixActiveNodeSet); !ani.isDone(); ani.next()) {
            // get the trie node and its distance
            TrieNode<charT> *trieNode;
            unsigned distance;
            ani.getItem(trieNode, distance);
	}
 */

template <class charT>
class ActiveNodeSetIterator
{
public:
    // generate an iterator for the active nodes whose edit distance is within the given @edUpperBound
    ActiveNodeSetIterator(PrefixActiveNodeSet<charT> *prefixActiveNodeSet, const unsigned edUpperBound)
    {
//        _initActiveNodeIterator(prefixActiveNodeSet, edUpperBound);
    }

    void next()
    {
        if (isDone())
            return;

        offsetCursor++;

        if (offsetCursor < trieNodeSetVector->at(editDistanceCursor).size())
            return;

        // reached the tail of the current vector
        editDistanceCursor++;
        offsetCursor = 0;

        // move editDistanceCursor to the next non-empty vector
        while (editDistanceCursor <= this->edUpperBound &&
               trieNodeSetVector->at(editDistanceCursor).size() == 0)
            editDistanceCursor++;
    }

    bool isDone()
    {
        if (editDistanceCursor <= this->edUpperBound &&
            offsetCursor < trieNodeSetVector->at(editDistanceCursor).size())
            return false;

        return true;
    }

    void getItem(unsigned long &trieNode, unsigned &distance)
    {
        if (isDone())
        {
            trieNode = NULL;
            distance = 0;
        }
        else
        {
            trieNode = trieNodeSetVector->at(editDistanceCursor).at(offsetCursor);
            distance = editDistanceCursor;
        }

        //ASSERT(distance != 0);
    }

private:
    typedef std::vector<unsigned long> TrieNodeSet;

    std::vector<TrieNodeSet> *trieNodeSetVector;
    unsigned edUpperBound;
    unsigned editDistanceCursor;
    unsigned offsetCursor;

    // initialize an iterator to store all the active nodes whose edit distance to
    // the query prefix is within the bound @edUpperBound
    void _initActiveNodeIterator(PrefixActiveNodeSet<charT> *prefixActiveNodeSet, const unsigned edUpperBound)
    {
        // we materialize the vector of trie nodes (indexed by the edit distance) only during the
        // phase of an iterator
//        this->trieNodeSetVector = prefixActiveNodeSet->getTrieNodeSetVector();
//
//        // ASSERT(edUpperBound < trieNodeSetVector->size());
//        this->edUpperBound = edUpperBound;
//
//        // initialize the cursors
//        this->editDistanceCursor = 0;
//        // Find the first valid active node
//        while (editDistanceCursor <= edUpperBound &&
//               trieNodeSetVector->at(editDistanceCursor).size() == 0)
//            editDistanceCursor++;
//
//        this->offsetCursor = 0;
    }
};

/*
 * An iterator to go through the leaf nodes of the active nodes in a PrefixActiveNodeSet.
 * An example:
 * PrefixActiveNodeSet<charT> *prefixActiveNodeSet;
 *         for (LeafNodeSetIterator<charT> lnsi(prefixActiveNodeSet); !lnsi.isDone(); lnsi.next()) {
            TrieNode<charT> *prefixNode;
            TrieNode<charT> *leafode;
            unsigned distance;
            ani.getItem(prefixNode, leafNode, distance);
	}
 */

// a structure to record information of each node
template <class charT>
struct LeafNodeSetIteratorItem
{
    unsigned long prefixNode;
    unsigned long leafNode;
    unsigned distance;

    // TODO: OPT. Return the length of the prefix instead of the prefixNode pointer?
    LeafNodeSetIteratorItem(unsigned long prefixNode, unsigned long leafNode, unsigned distance)
    {
        this->prefixNode = prefixNode;
        this->leafNode = leafNode;
        this->distance = distance;
    }
};

template <class charT>
class LeafNodeSetIterator
{
private:
    std::vector<LeafNodeSetIteratorItem<charT>> resultVector;
    unsigned cursor;

public:
    // for a set of active nodes, given a threshold edUpperBound, find
    // all the leaf nodes whose minimal edit distance (among all their prefixes)
    // is within the edUpperBound.
    // Provide an iterator that can return the leaf nodes sorted by their
    // minimal edit distance

    // Implementation: 1. Get the set of active nodes with an edit distance <= edUpperBound, sorted
    //                 based on their edit distance,
    //                 2. get their leaf nodes, and keep track of the visited nodes so that
    //                   each node is visited only once.
    LeafNodeSetIterator(PrefixActiveNodeSet<charT> *prefixActiveNodeSet, const unsigned edUpperBound, Trie *trie)
    {
//        _initLeafNodeSetIterator(prefixActiveNodeSet, edUpperBound, trie);
    }

    void next()
    {
        if (isDone())
            return;
        cursor++;
    }

    bool isDone()
    {
        //if (cursor >= leafNodesVector.size())
        if (cursor >= resultVector.size())
            return true;
        return false;
    }

    void getItem(unsigned long &prefixNode, unsigned long &leafNode, unsigned &distance)
    {
        if (isDone())
        {
            prefixNode = NULL;
            leafNode = NULL;
            distance = 0;
        }
        else
        {
            prefixNode = resultVector.at(cursor).prefixNode;
            leafNode = resultVector.at(cursor).leafNode;
            distance = resultVector.at(cursor).distance;
        }
    }

private:
    void _initLeafNodeSetIterator(PrefixActiveNodeSet<charT> *prefixActiveNodeSet, const unsigned edUpperBound, Trie *trie)
    {
//        std::set<unsigned long> visitedTrieNodes;
//
//        // assume the iterator returns the active nodes in an ascending order of their edit distance
//        for (ActiveNodeSetIterator<charT> ani(prefixActiveNodeSet, edUpperBound); !ani.isDone(); ani.next())
//        {
//            // get the trie node and its distance
//            unsigned long trieNode;
//            unsigned distance;
//            ani.getItem(trieNode, distance);
//
//            // append the leaf nodes of this active node to the vector
//            _appendLeafNodes(trieNode, trieNode, distance, visitedTrieNodes, trie);
//        }
//
//        // init the cursor
//        cursor = 0;
    }

    // add the leaf nodes of the given trieNode to a vector.  Add those decendant nodes to visitedTrieNodes.
    // Ignore those decendants that are already in visitedTrieNodes
    void _appendLeafNodes(unsigned long prefixNode, unsigned long trieNode, unsigned editDistance,
            std::set<unsigned long> &visitedTrieNodes, Trie *trie)
    {
//        // do nothing if this node has already been visited
//        if (visitedTrieNodes.find(trieNode) != visitedTrieNodes.end())
//            return;
//
//        // mark this node visited
//        visitedTrieNodes.insert(trieNode);
//
//        if (trie->getNode(trieNode).isTerminalNode())
//        {
//            // TODO: prefix might not be unique. Should we return the longest matching prefix?
//            resultVector.push_back(LeafNodeSetIteratorItem<charT>(prefixNode, trieNode, editDistance));
//        }
//
//        // go through the children
//        for (unsigned childIterator = 0; childIterator < this->trie.getNode(trieNode).getChildrenCount(); childIterator++)
//            _appendLeafNodes(prefixNode, this->trie.getNode(trieNode).getChild(childIterator), editDistance, visitedTrieNodes);
    }
};

#endif //__ACTIVENODE_H__
