#ifndef READ_H
#define READ_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

using namespace std;

class Read{
public:
	Read(string name, string seq, string strand, string quality);
	Read(string name, string seq, string strand);
	void print();

public:
	string mName;
	string mSeq;
	string mStrand;
	string mQuality;
	bool mHasQuality;
};

#endif