#include "mutation.h"
#include "editdistance.h"
#include <iostream>
#include <fstream>
#include "util.h"

Mutation::Mutation(string name, string left, string center, string right){
	//we shift 2 bases from left and right to center to require 100% match of these bases
    mLeft = left.substr(0, left.length()-2);
	mCenter = left.substr(left.length()-2, 2) + center + right.substr(0, 2);
    mRight = right.substr(2, right.length()-2);
    mPattern = left + center + right;
    mName = name;
}

int Mutation::searchInRead(Read* r, int distance){
    int readLen = r->mSeq.length();
    int lLen = mLeft.length();
    int cLen = mCenter.length();
    int rLen = mRight.length();
    int pLen = mPattern.length();
    string seq = r->mSeq.mStr;
    for(int start = lLen; start + cLen + rLen < readLen; start++){
        if(seq.substr(start, cLen) == mCenter){
            if (edit_distance(seq.substr(start - lLen, pLen), mPattern) <= distance){
                cout<<endl<<"Mutation: "<<mName<<endl;
                r->print();
            }
        }
    }
}

vector<Mutation> Mutation::parseFile(string filename) {
    ifstream file;
    file.open(filename.c_str(), ifstream::in);
    const int maxLine = 1000;
    char line[maxLine];
    vector<Mutation> mutations;
    while(file.getline(line, maxLine)){
        string linestr(line);
        vector<string> splitted;
        split(linestr, splitted, ",");
        // a valid line need 4 columns: name, left, center, right
        if(splitted.size()<4)
            continue;
        // comment line
        if(starts_with(splitted[0], "#"))
            continue;
        string name = trim(splitted[0]);
        string left = trim(splitted[1]);
        string center = trim(splitted[2]);
        string right = trim(splitted[3]);
        Mutation mut(name, left, center, right);
        mutations.push_back(mut);
    }
    file.close();
    return mutations;
}