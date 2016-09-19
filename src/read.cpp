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
	mHasQuality = r.mHasQuality;
}

int Read::length(){
	return mSeq.length();
}

void Read::print(){
	std::cout << mName << endl;
	std::cout << mSeq.mStr << endl;
	std::cout << mStrand << endl;
	if(mHasQuality)
		std::cout << mQuality << endl;
}

void Read::printWithBreaks(vector<int>& breaks){
	std::cout << mName << endl;
	std::cout << makeStringWithBreaks(mSeq.mStr, breaks)<< endl;
	std::cout << mStrand << endl;
	if(mHasQuality)
		std::cout << makeStringWithBreaks(mQuality, breaks) << endl;
}

string Read::makeStringWithBreaks(const string origin, vector<int>& breaks) {
	string ret = origin.substr(0, breaks[0]);
	for(int i=0;i<breaks.size()-1;i++){
		ret += " " + origin.substr(breaks[i], breaks[i+1]-breaks[i]);
	}
	if(breaks[breaks.size()-1]>0)
		ret += " " + origin.substr(breaks[breaks.size()-1], origin.length() - breaks[breaks.size()-1]);
	return ret;
}

Read* Read::reverseComplement(){
	Sequence seq = ~mSeq;
	string qual;
	qual.assign(mQuality.rbegin(), mQuality.rend());
	string strand = (mStrand=="+") ? "-" : "+";
	return new Read(mName, seq, strand, qual);
}