#ifndef READ_H
#define READ_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include "sequence.h"
#include <vector>

using namespace std;

class Read{
public:
	Read(string name, string seq, string strand, string quality);
    Read(string name, Sequence seq, string strand, string quality);
	Read(string name, string seq, string strand);
    Read(string name, Sequence seq, string strand);
    Read(Read &r);
	void print();
    void printFile(ofstream& file);
    Read* reverseComplement();
    string firstIndex();
    string lastIndex();
    // default is Q20
    int lowQualCount(int qual=20);
    int length();
    void printWithBreaks(vector<int>& breaks);
    void printHtmlTDWithBreaks(ofstream& file, vector<int>& breaks, int mutid, int matchid);
    void printJSWithBreaks(ofstream& file, vector<int>& breaks);

public:
    static bool test();

private:
    string makeStringWithBreaks(const string origin, vector<int>& breaks);
    string makeHtmlSeqWithQual(int start, int length);
    string qualityColor(char qual);

public:
	string mName;
	Sequence mSeq;
	string mStrand;
	string mQuality;
	bool mHasQuality;
};

class ReadPair{
public:
    ReadPair(Read* left, Read* right);
    ~ReadPair();

    // merge a pair, without consideration of seq error caused false INDEL
    Read* fastMerge();
public:
    Read* mLeft;
    Read* mRight;

public:
    static bool test();
};

#endif