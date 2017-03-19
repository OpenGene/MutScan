#ifndef MUTATION_H
#define MUTATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "read.h"
#include <vector>
#include "match.h"
#include <iostream>
#include <fstream>
#include "vcfreader.h"
#include "fastareader.h"

using namespace std;

class Mutation{
public:
    Mutation(string name, string left, string center, string right);

    //search this mutation in a read, and return the matchment info
    //by default, Q20 is required, and distance should be <=2
    //return NULL if not found
    Match* searchInRead(Read* r, int distanceReq = 2, int qualReq=20);
    static vector<Mutation> parseCsv(string filename);
    static vector<Mutation> parseBuiltIn();

    // if markedOnly = true, then only the entries with FILTER column = m will be treated
    // #CHROM   POS     ID          REF ALT QUAL  FILTER  INFO
    // 1        69224   COSM3677745 A   C   .     m       GENE=OR4F5;STRAND=+;CDS=c.134A>C;AA=p.D45A;CNT=1
    static vector<Mutation> parseVcf(string vcfFile, string refFile);
    void print();
    void printHtml(ofstream& file);
    string getCenterHtml();
    void setSmallIndel(bool flag);
    bool isSmallIndel();

public:
    string mLeft;
    string mCenter;
    string mRight;
    string mPattern;
    string mName;
    bool mSmallIndel;

    int mShift;
};


#endif
