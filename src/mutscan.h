#ifndef MUT_SCAN_H
#define MUT_SCAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "read.h"
#include "mutation.h"

using namespace std;

class MutScan{
public:
    MutScan(string mutationFile, string refFile, string read1File, string read2File, string html, string json, int threadNum);
    bool scan();
    static void evaluateSimplifiedMode(string r1file, string r2file, int mutationNum);

private:
    string mMutationFile;
    string mRead1File;
    string mRead2File;
    string mHtmlFile;
    string mJsonFile;
    string mRefFile;
    int mThreadNum;
    vector<Mutation> mutationList;
};


#endif