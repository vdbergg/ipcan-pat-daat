//
// Created by vdberg on 12/02/19.
//
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <sys/stat.h>
#include "../header/Trie.h"
#include "../header/C.h"
#include "../header/Framework.h"
#include "../header/Experiment.h"
#include "../header/Directives.h"
#include "../header/ActiveNodeIpcan.h"

using namespace std;

const unsigned MAX_UNSIGNED = 0xffffffff;

Framework::Framework(unordered_map<string,string> config) {
    this->trie = nullptr;
    this->editDistanceThreshold = stoi(config["edit_distance"]);
    this->dataset = stoi(config["dataset"]);
    this->experiment = new Experiment(config, editDistanceThreshold);
    this->config = config;

    index();
}

Framework::~Framework() {
    cout << "deleting framework" << endl;
    delete this->trie;
    delete this->experiment;
}

unsigned long getFileSize(string filename) {
    FILE *fp=fopen(filename.c_str(),"r");

    struct stat buf;
    fstat(fileno(fp), &buf);
    fclose(fp);
    return buf.st_size;
}

void Framework::readData(string& filename, vector<StaticString>& recs) {
    cout << "reading dataset " << filename << endl;

    string str;
    ifstream input(filename, ios::in);

    unsigned long fileSize = getFileSize(filename);
//    cout << "Tamanho do Arquivo:" << fileSize << endl;
    char *tmpPtr = (char*) malloc(sizeof(char)*fileSize);
    StaticString::setDataBaseMemory(tmpPtr,fileSize);
    while (getline(input, str)) {
//        for (char &c : str) {
//            if ((int) c == -61) continue;
//            else if ((int) c < 0 || (int) c >= CHAR_SIZE) {
//                c = utils::convertSpecialCharToSimpleChar(c);
//            }
//            c = tolower(c);
//        }
        if (!str.empty()) recs.push_back(StaticString(str));
    }
}

void Framework::readData(string& filename, vector<string>& recs) {
    cout << "reading dataset " << filename << endl;

    string str;
    ifstream input(filename, ios::in);
    while (getline(input, str)) {
//        for (char &c : str) {
//            if ((int) c == -61) continue;
//            else if ((int) c < 0 || (int) c >= CHAR_SIZE) {
//                c = utils::convertSpecialCharToSimpleChar(c);
//            }
//            c = tolower(c);
//        }
        if (!str.empty()) recs.push_back(str);
    }
}

void Framework::index() {
    cout << "indexing... \n";
    string sizeSufix = "";
    switch (stoi(config["size_type"])) {
        case 0:
            sizeSufix = "_25";
            break;
        case 1:
            sizeSufix = "_50";
            break;
        case 2:
            sizeSufix = "_75";
            break;
        case 3:
            sizeSufix = "";
            break;
        default:
            sizeSufix = "_60";
    }

    auto start = chrono::high_resolution_clock::now();
    #ifdef IS_COLLECT_TIME_H
        this->experiment->initIndexingTime();
    #endif

    string datasetFile = this->config["dataset_basepath"];
    string queryFile = this->config["query_basepath"];
    string relevantQueryFile = this->config["query_basepath"];

    int queriesSize = stoi(this->config["queries_size"]);
    string datasetSuffix = queriesSize == 10 ? "_10" : "";
    string tau = to_string(this->editDistanceThreshold);

    switch (this->dataset) {
        case C::AOL:
            datasetFile += "aol/aol" + sizeSufix + ".txt";
            queryFile += "aol/q17_" + tau + datasetSuffix + ".txt";
            break;
        case C::MEDLINE:
            datasetFile += "medline/medline" + sizeSufix + ".txt";
            queryFile += "medline/q13" + datasetSuffix + ".txt";
            break;
        case C::USADDR:
            datasetFile += "usaddr/usaddr" + sizeSufix + ".txt";
            queryFile += "usaddr/q17_" + tau + datasetSuffix + ".txt";
            break;
        case C::MEDLINE19:
            datasetFile += "medline19/medline19" + sizeSufix + ".txt";
            queryFile += "medline19/q17_" + tau + datasetSuffix + ".txt";
            break;
        case C::DBLP:
            datasetFile += "dblp/dblp" + sizeSufix + ".txt";
            queryFile += "dblp/q17_" + tau + datasetSuffix + ".txt";
            break;
        case C::UMBC:
            datasetFile += "umbc/umbc" + sizeSufix + ".txt";
            queryFile += "umbc/q17_" + tau + datasetSuffix + ".txt";
            break;
        case C::JUSBRASIL:
            datasetFile += "jusbrasil/jusbrasil" + sizeSufix + ".txt";
            queryFile += "jusbrasil/q.txt";
            relevantQueryFile += "jusbrasil/relevant_answers.txt";
            break;
        default:
            datasetFile += "aol/aol" + sizeSufix + ".txt";
            queryFile += "aol/q17_" + tau + datasetSuffix + ".txt";
            break;
    }

    readData(datasetFile, records);
    readData(queryFile, this->queries);

    if (this->config["has_relevant_queries"] == "1") {
        readData(relevantQueryFile, this->relevantQueries);
    }

    this->trie = new Trie(this->experiment);
    this->trie->buildDaatIndex();
    this->trie->shrinkToFit();

    #ifdef IS_COLLECT_MEMORY_H
        this->experiment->getMemoryUsedInIndexing();
    #else
        this->experiment->compileProportionOfBranchingSizeInBEVA2Level();
        this->experiment->endIndexingTime();
        this->experiment->compileNumberOfNodes();
    #endif
    auto done = chrono::high_resolution_clock::now();
    cout << "<<<Index time: "<< chrono::duration_cast<chrono::milliseconds>(done - start).count() << " ms>>>\n";
}

unordered_map<int, char*> Framework::processFullQuery(string &query, int queryId) {
    #ifdef IS_COLLECT_TIME_H
        this->experiment->initQueryProcessingTime();
    #endif

    PrefixActiveNodeSet<char>* pset = new PrefixActiveNodeSet<char>(this->trie, this->editDistanceThreshold);

    for (int currentPrefixQuery = 1; currentPrefixQuery <= query.size(); currentPrefixQuery++) {
        PrefixActiveNodeSet<char>* temp = pset->computeActiveNodeSetIncrementally(query[currentPrefixQuery - 1], this->trie);
        delete pset;
        pset = temp;
        map<CustomTrieNode, unsigned> minActiveNodes;
        pset->computeMinimizedTrieNodesInRange(0, this->editDistanceThreshold, minActiveNodes,trie);
    }

    #ifdef IS_COLLECT_TIME_H
        this->experiment->endSimpleQueryProcessingTime(pset->getNumberOfActiveNodes());
        this->experiment->initQueryFetchingTime();
    #endif

    unordered_map<int, char*> results = this->output(pset);

    #ifdef IS_COLLECT_TIME_H
        this->experiment->endSimpleQueryFetchingTime(results.size());

        bool relevantReturned = false;
        if (queryId != -1 && this->config["has_relevant_queries"] == "1") {
            vector<string> v_output;
            v_output.reserve(results.size());
            for (auto it = results.begin(); it != results.end(); it++) {
                v_output.emplace_back(it->second);
            }

	        relevantReturned = find(v_output.begin(), v_output.end(),
	                this->relevantQueries[queryId]) != v_output.end();
        }
        this->experiment->compileSimpleQueryProcessingTimes(query, relevantReturned);
    #endif

    #ifdef IS_COLLECT_MEMORY_H
        this->experiment->getMemoryUsedInProcessing();
    #endif

    return results;
}

int count1 = 0;

unordered_map<int, char*> Framework::processQuery(string &query, int queryId) {
    PrefixActiveNodeSet<char>* pset = new PrefixActiveNodeSet<char>(this->trie, this->editDistanceThreshold);
    string currentQuery = "";
    unordered_map<int, char*> results;

    for (int currentPrefixQuery = 1; currentPrefixQuery <= query.size(); currentPrefixQuery++) {
        #ifdef IS_COLLECT_TIME_H
            this->experiment->initQueryProcessingTime();
        #endif

        char ch = queries[queryId][currentPrefixQuery - 1];     // current key stroke
        currentQuery += ch;

        PrefixActiveNodeSet<char>* temp = pset->computeActiveNodeSetIncrementally(ch,this->trie);
        delete pset;
        pset = temp;
        map<CustomTrieNode, unsigned> minActiveNodes;
        pset->computeMinimizedTrieNodesInRange(0, this->editDistanceThreshold, minActiveNodes, this->trie);

        #ifdef IS_COLLECT_TIME_H
            experiment->endQueryProcessingTime(pset->getNumberOfActiveNodes(), currentQuery.size());
        #endif

        vector<int> prefixQuerySizeToFetching = { 2, 3, 4, 5, 6, 7, 8, 9, 13, 17 };

        if (std::find(prefixQuerySizeToFetching.begin(), prefixQuerySizeToFetching.end(), currentQuery.size()) !=
            prefixQuerySizeToFetching.end()) {
            #ifdef IS_COLLECT_TIME_H
                experiment->initQueryFetchingTime();
            #endif

            results = this->output(pset);

            #ifdef IS_COLLECT_TIME_H
                experiment->endQueryFetchingTime(currentQuery.size(), results.size());
            #endif
        }

        if (currentQuery.length() == queries[queryId].length()) {
            #ifdef IS_COLLECT_TIME_H
                experiment->compileQueryProcessingTimes(queryId);
                experiment->saveQueryProcessingTime(currentQuery, queryId);
            #endif

            #ifdef IS_COLLECT_MEMORY_H
                this->experiment->getMemoryUsedInProcessing();
            #endif
        }
    }

    return results;
}

unordered_map<int, char*> Framework::output(PrefixActiveNodeSet<char>* pset) {
    unordered_map<int, char*> outputs;

    for (auto mit = pset->PANMap.begin(); mit != pset->PANMap.end(); mit++) {
        unsigned beginRange = this->trie->getNode(mit->first.trieNode).getBeginRange();
        unsigned endRange = this->trie->getNode(mit->first.trieNode).getEndRange();

        for (unsigned i = beginRange; i < endRange; i++) {
            outputs[i] = records[i].c_str();
        }
    }

    return outputs;
}

void Framework::writeExperiments() {
    #ifdef IS_COLLECT_COUNT_OPERATIONS_H
        this->experiment->compileNumberOfIterationInChildren();
        this->experiment->compileNumberOfActiveNodes();
        this->experiment->compileNumberOfWordActiveNodes();
        this->experiment->compileNumberOfIterationInChildren2Level();
    #endif
}
