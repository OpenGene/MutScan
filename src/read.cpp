#include "read.h"
#include <sstream>

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

void Read::print(){
	std::cout << mName << endl;
	std::cout << mSeq.mStr << endl;
	std::cout << mStrand << endl;
	if(mHasQuality)
		std::cout << mQuality << endl;
}

void Read::printFile(ofstream& file){
	file << mName << endl;
	file << mSeq.mStr << endl;
	file << mStrand << endl;
	if(mHasQuality)
		file << mQuality << endl;
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

void Read::printHtmlTDWithBreaks(ofstream& file, vector<int>& breaks) {
	file << "<td class='alignright'>" << makeHtmlSeqWithQual(0, breaks[0]) << "</td>";
	for(int i=0;i<breaks.size()-1;i++){
		file << "<td>" << makeHtmlSeqWithQual(breaks[i], breaks[i+1]-breaks[i]) << "</td>";
	}
	if(breaks[breaks.size()-1]>0)
		file << "<td class='alignleft'>" << makeHtmlSeqWithQual(breaks[breaks.size()-1], mSeq.mStr.length() - breaks[breaks.size()-1]) << "</td>";
}

string Read::makeHtmlSeqWithQual(int start, int length) {
    stringstream ss;
	for(int i=start;i<start+length && i<mSeq.length(); i++) {
		ss << "<a title='" << mQuality[i] << "'><font color='" << qualityColor(mQuality[i]) << "'>"<< mSeq.mStr[i] << "</font></a>";
	}
	return ss.str();
}

string Read::qualityColor(char qual) {
	if(qual >= 'A') // >= Q32, extremely high quality
		return "#78C6B9";
	if(qual >= ';') // Q26 ~ Q31, high quality
		return "#33BBE2";
	if(qual >= '5') // Q20 ~ Q25, moderate quality
		return "#666666";
	if(qual >= '0') // Q15 ~ Q19, low quality
		return "#E99E5B";
	else // <= Q14, extremely low quality
		return "#FF0000";
}

Read* Read::reverseComplement(){
	Sequence seq = ~mSeq;
	string qual;
	qual.assign(mQuality.rbegin(), mQuality.rend());
	string strand = (mStrand=="+") ? "-" : "+";
	return new Read(mName, seq, strand, qual);
}

string Read::lastIndex(){
	int len = mName.length();
	if(len<5)
		return "";
	for(int i=len-5;i>=0;i--){
		if(mName[i]==':' or mName[i]=='+'){
			return mName.substr(i+1, len-i);
		}
	}
}

int Read::lowQualCount(int qual){
	int count = 0;
	for(int q=0;q<mQuality.size();q++){
		if(mQuality[q] < qual + 33)
			count++;
	}
	return count;
}

int Read::length(){
	return mSeq.length();
}

bool Read::test(){
	Read r("@NS500713:64:HFKJJBGXY:1:11101:20469:1097 1:N:0:TATAGCCT+GGTCCCGA",
		"CTCTTGGACTCTAACACTGTTTTTTCTTATGAAAACACAGGAGTGATGACTAGTTGAGTGCATTCTTATGAGACTCATAGTCATTCTATGATGTAGTTTTCCTTAGGAGGACATTTTTTACATGAAATTATTAACCTAAATAGAGTTGATC",
		"+",
		"AAAAA6EEEEEEEEEEEEEEEEE#EEEEEEEEEEEEEEEEE/EEEEEEEEEEEEEEEEAEEEAEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE<EEEEAEEEEEEEEEEEEEEEAEEE/EEEEEEEEEEAAEAEAAEEEAEEAA");
	string idx = r.lastIndex();
	return idx == "GGTCCCGA";
}

ReadPair::ReadPair(Read* left, Read* right){
	mLeft = left;
	mRight = right;
}

ReadPair::~ReadPair(){
	if(mLeft){
		delete mLeft;
		mLeft = NULL;
	}
	if(mRight){
		delete mRight;
		mRight = NULL;
	}
}