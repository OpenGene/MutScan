#include "read.h"

Read::Read(string name, string seq, string strand, string quality){
	mName = name;
	mSeq = Sequence(seq);
	mStrand = strand;
	mQuality = quality;
	mHasQuality = true;
}

Read::Read(string name, string seq, string strand){
	mName = name;
	mSeq = Sequence(seq);
	mStrand = strand;
	mHasQuality = false;
}

Read::Read(string name, Sequence seq, string strand, string quality){
	mName = name;
	mSeq = seq;
	mStrand = strand;
	mQuality = quality;
	mHasQuality = true;
}

Read::Read(string name, Sequence seq, string strand){
	mName = name;
	mSeq = seq;
	mStrand = strand;
	mHasQuality = false;
}

void Read::print(){
	std::cout << mName;
	std::cout << mSeq.mStr;
	std::cout << mStrand;
	if(mHasQuality)
		std::cout << mQuality;
}