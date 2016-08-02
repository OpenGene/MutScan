#ifndef MUTATION_H
#define MUTATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "read.h"

using namespace std;

class Mutation{
public:
    Mutation(string left, string center, string right, string name);

    //search this mutation in a read, and return the left pos where this mutation pattern starts
    //return -1 if not found
    int searchInRead(Read* r, int distance = 2);

private:
    string mLeft;
    string mCenter;
    string mRight;
    string mPattern;
    string mName;
};


#endif