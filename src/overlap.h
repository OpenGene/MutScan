#ifndef OVERLAP_H
#define OVERLAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "sequence.h"

using namespace std;

class Overlap{
public:
    Overlap(int offset, int overlapLen, int distance);
    static Overlap fit(Sequence R1, Sequence R2);

public:
    int mOffset;
    int mOverlapLen;
    int mDistance;
    bool mOverlapped;
};

#endif