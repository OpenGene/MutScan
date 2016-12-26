#ifndef HTML_REPORTER_H
#define HTML_REPORTER_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "read.h"
#include "mutation.h"
#include <iostream>
#include <fstream>

using namespace std;

class HtmlReporter{
public:
    HtmlReporter(string filename, vector<Mutation>& mutationList, vector<Match*> *mutationMatches);
    ~HtmlReporter();
    void run();

private:
    void printHeader();
    void printCSS();
    void printJS();
    void printFooter();
    void printHelper();
    void printMutations();
    void printMutation(int id, Mutation& mutation, vector<Match*>& matches);
    void printScanTargets();

private:
    string mFilename;
    vector<Mutation> mMutationList;
    vector<Match*>* mMutationMatches;
    ofstream mFile;
};


#endif