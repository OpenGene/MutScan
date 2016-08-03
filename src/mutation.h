#ifndef MUTATION_H
#define MUTATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "read.h"
#include <vector>

using namespace std;

class Mutation{
public:
    Mutation(string name, string left, string center, string right);

    //search this mutation in a read, and return the left pos where this mutation pattern starts
    //by default, Q20 is required, and distance should be <=2
    //return -1 if not found
    int searchInRead(Read* r, int distanceReq = 2, int qualReq=20);
    static vector<Mutation> parseFile(string filename);

private:
    string mLeft;
    string mCenter;
    string mRight;
    string mPattern;
    string mName;
};


#endif