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
    MutScan(string mutationFile, string read1File, string read2File, string html);
    bool scan();
    void textReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches);
    void htmlReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches);

private:
    string mMutationFile;
    string mRead1File;
    string mRead2File;
    string mHtmlFile;
};


#endif