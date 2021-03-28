//
// Created by vdberg on 12/02/19.
//

#ifndef BEVA_FRAMEWORK_H
#define BEVA_FRAMEWORK_H

#include <vector>
#include <string>
#include "Trie.h"
#include "Experiment.h"
#include "ActiveNodeIpcan.h"

using namespace std;

class Framework {
public:
    Trie* trie;
    vector<string> queries;
    vector<string> relevantQueries;
    int editDistanceThreshold;
    int dataset;
    Experiment* experiment;
    unordered_map<string, string> config;

    Framework(unordered_map<string,string>);

    void readData(string&, vector<string>&);
    void readData(string&, vector<StaticString>&);
    void index();
    unordered_map<int, char*> processFullQuery(string &query, int queryId = -1);
    unordered_map<int, char*> processQuery(string &query, int queryId);
    unordered_map<int, char*> output(PrefixActiveNodeSet<char>* pset);

    ~Framework();

    void writeExperiments();
};


#endif //BEVA_FRAMEWORK_H
