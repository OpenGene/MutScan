#include "mutation.h"
#include "editdistance.h"
#include <iostream>
#include <fstream>
#include "util.h"
#include <string.h>

Mutation::Mutation(string name, string left, string center, string right){
	//we shift some bases from left and right to center to require 100% match of these bases
    mLeft = left.substr(0, left.length()-mShift);
	mCenter = left.substr(left.length()-mShift, mShift) + center + right.substr(0, mShift);
    mRight = right.substr(mShift, right.length()-mShift);
    mPattern = left + center + right;
    mName = name;
}

Match* Mutation::searchInRead(Read* r, int distanceReq, int qualReq){
    char phredQualReq= (char)(qualReq + 33);
    int readLen = r->mSeq.length();
    int lLen = mLeft.length();
    int cLen = mCenter.length();
    int rLen = mRight.length();
    int pLen = mPattern.length();
    string seq = r->mSeq.mStr;
    const char* seqData = seq.c_str();
    const char* centerData = mCenter.c_str();
    const char* patternData = mPattern.c_str();
    const char* qualData = r->mQuality.c_str();
    for(int start = lLen; start + cLen + rLen < readLen; start++){
        // check string identity in a fast way
        bool identical = true;
        for (int i=0;i<cLen;i++){
            if (seqData[start + i] != centerData[i]){
                identical = false;
                break;
            }
        }
        if(!identical)
            continue;

        // check quality in a fast way
        bool qualityPassed = true;
        for (int i=mShift; i<cLen-mShift; i++){
            if (qualData[start + i] < phredQualReq){
                qualityPassed = false;
                break;
            }
        }
        if(!qualityPassed)
            continue;
        int ed = edit_distance(seqData + start - lLen, pLen, patternData, pLen);
        if ( ed <= distanceReq){
            return new Match(r, start-lLen, ed);
        }
    }
    return NULL;
}

vector<Mutation> Mutation::parseFile(string filename) {
    ifstream file;
    file.open(filename.c_str(), ifstream::in);
    const int maxLine = 1000;
    char line[maxLine];
    vector<Mutation> mutations;
    while(file.getline(line, maxLine)){
        // trim \n, \r or \r\n in the tail
        int readed = strlen(line);
        if(readed >=2 ){
            if(line[readed-1] == '\n' || line[readed-1] == '\r'){
                line[readed-1] = '\0';
                if(line[readed-2] == '\r')
                    line[readed-2] = '\0';
            }
        }
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

void Mutation::print(){
    cout<<mName<<" "<<mLeft<<" "<<mCenter<<" "<<mRight<<endl;
}