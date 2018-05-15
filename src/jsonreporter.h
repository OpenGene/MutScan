#ifndef JSON_REPORTER_H
#define JSON_REPORTER_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "read.h"
#include "mutation.h"
#include <iostream>
#include <fstream>

using namespace std;

class JsonReporter{
public:
    JsonReporter(string filename, vector<Mutation>& mutationList, vector<Match*> *mutationMatches);
    ~JsonReporter();
    void run();

private:
    void printHeader();
    void printCSS();
    void printJS();
    void printFooter();
    void printHelper();
    void printMutations();
    void printMutationsJS();
    void printMutation(int id, Mutation& mutation, vector<Match*>& matches);
    void printScanTargets();

private:
    string mFilename;
    vector<Mutation> mMutationList;
    vector<Match*>* mMutationMatches;
    ofstream mFile;
};

#endif