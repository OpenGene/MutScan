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

Read::Read(Read &r) {
	mName = r.mName;
	mSeq = r.mSeq;
	mStrand = r.mStrand;
	mQuality = r.mQuality;
}

void Read::print(){
	std::cout << mName << endl;
	std::cout << mSeq.mStr << endl;
	std::cout << mStrand << endl;
	if(mHasQuality)
		std::cout << mQuality << endl;
}

Read* Read::reverseComplement(){
	Sequence seq = ~mSeq;
	string qual;
	qual.assign(mQuality.rbegin(), mQuality.rend());
	string strand = (mStrand=="+") ? "-" : "+";
	return new Read(mName, seq, strand, qual);
}