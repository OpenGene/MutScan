#include "match.h"
#include <vector>
#include "globalsettings.h"
#include <memory.h>

Match::Match(Read* r, int pos, int distance, bool reversed){
    mRead = r;
    mReadLen = r->length();
    mSequence = NULL;
    mDistance = distance;
    mPos = pos;
    mReversed = reversed;
    if(GlobalSettings::simplifiedMode)
        mOriginalReads = NULL;
    else
        mOriginalReads = new vector<Read*>();
}

Match::Match(char* seq, int readLen, char meanQual, int pos, int distance, bool reversed){
    mRead = NULL;
    mSequence = seq;
    mReadLen = readLen;
    mMeanQual = meanQual;
    mDistance = distance;
    mPos = pos;
    mReversed = reversed;
    if(GlobalSettings::simplifiedMode)
        mOriginalReads = NULL;
    else
        mOriginalReads = new vector<Read*>();
}

Match::~Match(){
    // we don't delete mRead or mSequence here since they are shared by different objects
    // and will be deleted in other places
    if(mOriginalReads) {
        for(int i=0;i<mOriginalReads->size();i++){
            delete (*mOriginalReads)[i];
            (*mOriginalReads)[i] = NULL;
        }
        delete mOriginalReads;
        mOriginalReads = NULL;
    }
}

int Match::readlength() const {
    return mReadLen;
}

void Match::addOriginalRead(Read* r){
    if(!mOriginalReads)
        return;
    mOriginalReads->push_back(new Read(*r));
}

void Match::addOriginalPair(ReadPair* pair){
    if(!mOriginalReads)
        return;
    mOriginalReads->push_back(new Read(*pair->mLeft));
    mOriginalReads->push_back(new Read(*pair->mRight));
}

void Match::print(int leftlen, int centerlen, int rightlen){
    if(GlobalSettings::simplifiedMode)
        mRead = new Read(mSequence, mReadLen, mMeanQual);
    cout<<"pos: "<<mPos<<", distance: "<<(int)mDistance;
    if(mReversed)
        cout<<", reverse";
    else
        cout<<", forward";
    cout<<endl;
    vector<int> breaks;
    breaks.push_back(max(mPos-leftlen, 0));
    breaks.push_back( mPos );
    breaks.push_back( mPos+centerlen );
    breaks.push_back( min(mPos+centerlen+rightlen, mRead->length()));
    mRead->printWithBreaks(breaks);
    if(GlobalSettings::simplifiedMode) {
        delete mRead;
        mRead = NULL;
    }
}

void Match::printHtmlTD(ofstream& file, int leftlen, int centerlen, int rightlen, int mutid, int matchid){
    if(GlobalSettings::simplifiedMode)
        mRead = new Read(mSequence, mReadLen, mMeanQual);
    file<<"<a title='"<<mRead->mName<<"'>";
    file<<"d:" << (int)mDistance;
    if(mReversed)
        file<<", <--";
    else
        file<<", -->";

    file<<"</a></span>";

    vector<int> breaks;
    breaks.push_back(max(mPos-leftlen, 0));
    breaks.push_back( mPos );
    breaks.push_back( mPos+centerlen );
    breaks.push_back( min(mPos+centerlen+rightlen, mRead->length()));
    mRead->printHtmlTDWithBreaks(file, breaks, mutid, matchid);
    if(GlobalSettings::simplifiedMode) {
        delete mRead;
        mRead = NULL;
    }
}

void Match::printBreaksToJson(ofstream& file, int leftlen, int centerlen, int rightlen){
    vector<int> breaks;
    breaks.push_back(max(mPos-leftlen, 0));
    breaks.push_back( mPos );
    breaks.push_back( mPos+centerlen );
    breaks.push_back( min(mPos+centerlen+rightlen, mReadLen));
    file << "[";
    for(int i=0; i<breaks.size(); i++) {
        file << breaks[i];
        if(i!=breaks.size() - 1) 
            file << ",";
    }
    file << "]";
}

void Match::printReadToJson(ofstream& file, string pad) {
    if(GlobalSettings::simplifiedMode)
        mRead = new Read(mSequence, mReadLen, mMeanQual);
    file << pad << "\"seq\":" << "\"" <<  mRead->mSeq.mStr << "\"," << endl;
    file << pad << "\"qual\":" << "\"" <<  mRead->mQuality << "\"" << endl;
    if(GlobalSettings::simplifiedMode) {
        delete mRead;
        mRead = NULL;
    }
}

void Match::printJS(ofstream& file, int leftlen, int centerlen, int rightlen) {
    if(GlobalSettings::simplifiedMode)
        mRead = new Read(mSequence, mReadLen, mMeanQual);
    vector<int> breaks;
    breaks.push_back(max(mPos-leftlen, 0));
    breaks.push_back( mPos );
    breaks.push_back( mPos+centerlen );
    breaks.push_back( min(mPos+centerlen+rightlen, mRead->length()));
    mRead->printJSWithBreaks(file, breaks);
    if(GlobalSettings::simplifiedMode) {
        delete mRead;
        mRead = NULL;
    }
}

void Match::printReadsToFile(ofstream& file){
    if(!mOriginalReads)
        return;
    for(int i=0;i<mOriginalReads->size();i++){
        (*mOriginalReads)[i]->printFile(file);
    }
}

void Match::setReversed(bool flag){
    mReversed = flag;
}

int Match::countUnique(vector<Match*>& matches) {
    if(matches.size()==0)
        return 0;
    int count = 1;
    Match* cur = matches[0];
    for(int i=1;i<matches.size();i++){
        Match* m = matches[i];
        if( *m > *cur || *m < *cur) {
            cur = m;
            count++;
        }
    }
    return count;
}
