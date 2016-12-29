#ifndef MATCH_H
#define MATCH_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "read.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

class Match{
public:
    Match(Read* r, int pos, int distance, bool reversed = false);
    ~Match();
    void print(int leftlen, int centerlen, int rightlen);
    void printHtmlTD(ofstream& file, int leftlen, int centerlen, int rightlen);
    void printReadsToFile(ofstream& file);
    void setReversed(bool flag);
    void addOriginalRead(Read* r);
    void addOriginalPair(ReadPair* pair);

private:
    Read* mRead;
    vector<Read*> mOriginalReads;
    int mDistance;
    bool mReversed;
    // the start position of the mutation's center
    int mPos;
};


#endif