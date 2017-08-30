#include "read.h"
#include <sstream>
#include "util.h"

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

void Read::printHtmlTDWithBreaks(ofstream& file, vector<int>& breaks, int mutid, int matchid) {
	file << "<td id='b-"<<mutid<<"-"<<matchid<<"-"<<"0' class='alignright'>" << makeHtmlSeqWithQual(0, breaks[0]) << "</td>";
	for(int i=0;i<breaks.size()-1;i++){
		file << "<td id='b-"<<mutid<<"-"<<matchid<<"-"<<i+1<<"' ";
		if(i==0)
			file << " class='alignright'";
		file << ">" << makeHtmlSeqWithQual(breaks[i], breaks[i+1]-breaks[i]) << "</td>";
	}
	if(breaks[breaks.size()-1]>0)
		file << "<td id='b-"<<mutid<<"-"<<matchid<<"-"<<breaks.size()<<"' ";
		file << "class='alignleft'>" << makeHtmlSeqWithQual(breaks[breaks.size()-1], mSeq.mStr.length() - breaks[breaks.size()-1]) << "</td>";
}

string Read::makeHtmlSeqWithQual(int start, int length) {
	//colorful HTML report is dynamically created by JavaScript
	//here we return gray report first
	return mSeq.mStr.substr(start, length);

    /*stringstream ss;
	for(int i=start;i<start+length && i<mSeq.length(); i++) {
		ss << "<a title='" << mQuality[i] << "'><font color='" << qualityColor(mQuality[i]) << "'>"<< mSeq.mStr[i] << "</font></a>";
	}
	return ss.str();*/
}

void Read::escapeSpecialQualityChar(string& str) {
	const char* data = str.c_str();
	for(int i=0;i<str.length();i++){
		if(data[i] == '\'' || data[i] == '\\')
			str[i] = data[i] + 1;
	}
}

void Read::printJSWithBreaks(ofstream& file, vector<int>& breaks) {
	if(breaks.size()>0){
		file << "\n[";
		file << "'" << mSeq.mStr.substr(0, breaks[0]) << "'";
		file << ", " ;
		string qualstr = mQuality.substr(0, breaks[0]);
		escapeSpecialQualityChar(qualstr);
		file << "'" << qualstr << "'";
		file << "],";
	}
	for(int i=0;i<breaks.size()-1;i++){
		file << "\n[";
		file << "'" << mSeq.mStr.substr(breaks[i], breaks[i+1]-breaks[i]) << "'";
		file << ", " ;
		string qualstr = mQuality.substr(breaks[i], breaks[i+1]-breaks[i]);
		escapeSpecialQualityChar(qualstr);
		file << "'" << qualstr << "'";
		file << "],";
	}
	if(breaks[breaks.size()-1]>0){
		file << "\n[";
		file << "'" << mSeq.mStr.substr(breaks[breaks.size()-1], mSeq.mStr.length() - breaks[breaks.size()-1]) << "'";
		file << ", " ;
		string qualstr = mQuality.substr(breaks[breaks.size()-1], mSeq.mStr.length() - breaks[breaks.size()-1]);
		escapeSpecialQualityChar(qualstr);
		file << "'" << qualstr << "'";
		file << "],";
	}

}

string Read::qualityColor(char qual) {
	if(qual >= 'I') // >= Q40, extremely high quality
		return "#78C6B9";
	if(qual >= '?') // Q30 ~ Q39, high quality
		return "#33BBE2";
	if(qual >= '5') // Q20 ~ Q29, moderate quality
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
		if(mName[i]==':' || mName[i]=='+'){
			return mName.substr(i+1, len-i);
		}
	}
	return "";
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

Read* ReadPair::fastMerge(){
	Read* rcRight = mRight->reverseComplement();
	int len1 = mLeft->length();
	int len2 = rcRight->length();
	// use the pointer directly for speed
	const char* str1 = mLeft->mSeq.mStr.c_str();
	const char* str2 = rcRight->mSeq.mStr.c_str();
	const char* qual1 = mLeft->mQuality.c_str();
	const char* qual2 = rcRight->mQuality.c_str();

	// we require at least 30 bp overlapping to merge a pair
	const int MIN_OVERLAP = 30;
	bool overlapped = false;
	int olen = MIN_OVERLAP;
	int diff = 0;
	// the diff count for 1 high qual + 1 low qual
	int lowQualDiff = 0;

	while(olen <= min(len1, len2)){
		diff = 0;
		lowQualDiff = 0;
		bool ok = true;
		int offset = len1 - olen;
		for(int i=0;i<olen;i++){
			if(str1[offset+i] != str2[i]){
				diff++;
				// one is >= Q30 and the other is <= Q15
				if((qual1[offset+i]>='?' && qual2[i]<='0') || (qual1[offset+i]<='0' && qual2[i]>='?')){
					lowQualDiff++;
				}
				// we disallow high quality diff, and only allow up to 3 low qual diff
				if(diff>lowQualDiff || lowQualDiff>=3){
					ok = false;
					break;
				}
			}
		}
		if(ok){
			overlapped = true;
			break;
		}
		olen++;
	}

	if(overlapped){
		int offset = len1 - olen;
		stringstream ss;
		ss << mLeft->mName << " merged offset:" << offset << " overlap:" << olen << " diff:" << diff;
		string mergedName = ss.str();
		string mergedSeq = mLeft->mSeq.mStr.substr(0, offset) + rcRight->mSeq.mStr;
		string mergedQual = mLeft->mQuality.substr(0, offset) + rcRight->mQuality;
		// quality adjuction and correction for low qual diff
		for(int i=0;i<olen;i++){
			if(str1[offset+i] != str2[i]){
				if(qual1[offset+i]>='?' && qual2[i]<='0'){
					mergedSeq[offset+i] = str1[offset+i];
					mergedQual[offset+i] = qual1[offset+i];
				} else {
					mergedSeq[offset+i] = str2[i];
					mergedQual[offset+i] = qual2[i];
				}
			} else {
				// add the quality of the pair to make a high qual
				mergedQual[offset+i] =  qual1[offset+i] + qual2[i] - 33;
			}
		}
		delete rcRight;
		return new Read(mergedName, mergedSeq, "+", mergedQual);
	}

	delete rcRight;
	return NULL;
}

bool ReadPair::test(){
	Read* left = new Read("@NS500713:64:HFKJJBGXY:1:11101:20469:1097 1:N:0:TATAGCCT+GGTCCCGA",
		"TTTTTTCTCTTGGACTCTAACACTGTTTTTTCTTATGAAAACACAGGAGTGATGACTAGTTGAGTGCATTCTTATGAGACTCATAGTCATTCTATGATGTAG",
		"+",
		"AAAAA6EEEEEEEEEEEEEEEEE#EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEAEEEAEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE");
	Read* right = new Read("@NS500713:64:HFKJJBGXY:1:11101:20469:1097 1:N:0:TATAGCCT+GGTCCCGA",
		"AAAAAACTACACCATAGAATGACTATGAGTCTCATAAGAATGCACTCAACTAGTCATCACTCCTGTGTTTTCATAAGAAAAAACAGTGTTAGAGTCCAAGAG",
		"+",
		"AAAAA6EEEEE/EEEEEEEEEEE#EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEAEEEAEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE");

	ReadPair pair(left, right);
	Read* merged = pair.fastMerge();
	if(merged == NULL)
		return false;

	if(merged->mSeq.mStr != "TTTTTTCTCTTGGACTCTAACACTGTTTTTTCTTATGAAAACACAGGAGTGATGACTAGTTGAGTGCATTCTTATGAGACTCATAGTCATTCTATGATGTAGTTTTTT")
		return false;

	//merged->print();
	return true;
}
