#ifndef MATCH_H
#define MATCH_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "read.h"

using namespace std;

class Match{
public:
    Match(Read* r1, Read* r2, int distance, int pos, int type, bool reversed);
    ~Match();
    enum {
        Read1,
        Read2,
        Merged
    };

private:
    Read* mRead1;
    Read* mRead2;
    int mDistance;
    int mType;
    bool mReversed;
    int mPos;
};


#endif