#ifndef READ_H
#define READ_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include "sequence.h"

using namespace std;

class Read{
public:
	Read(string name, string seq, string strand, string quality);
    Read(string name, Sequence seq, string strand, string quality);
	Read(string name, string seq, string strand);
    Read(string name, Sequence seq, string strand);
    Read(Read &r);
	void print();
    Read* reverseComplement();

public:
	string mName;
	Sequence mSeq;
	string mStrand;
	string mQuality;
	bool mHasQuality;
};

#endif