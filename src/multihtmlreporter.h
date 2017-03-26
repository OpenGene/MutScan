#ifndef MULTI_HTML_REPORTER_H
#define MULTI_HTML_REPORTER_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "read.h"
#include "mutation.h"
#include <iostream>
#include <fstream>

using namespace std;

class MultiHtmlReporter{
public:
    MultiHtmlReporter(string filename, vector<Mutation>& mutationList, vector<Match*> *mutationMatches);
    ~MultiHtmlReporter();
    void run();

private:
    void printMainFrame();
    void printMainPage();
    void printIndexPage();
    void printAllChromosomeLink(ofstream& file);
    void printChrLink(ofstream& file, string chr);
    void printChrHtml();
    void printMutationHtml();
    void stat();
    void printHeader(ofstream& file);
    void printCSS(ofstream& file);
    void printJS(ofstream& file);
    void printFooter(ofstream& file, bool printTargetList=true);
    void printHelper(ofstream& file);
    void printScanTargets(ofstream& file);
    string getCurrentSystemTime();

private:
    string mFilename;
    string mFolderName;
    vector<Mutation> mMutationList;
    vector<Match*>* mMutationMatches;
    map<string, int> mChrCount;
    int mTotalCount;
};


#endif