#include "match.h"
#include <vector>

Match::Match(Read* r, int pos, int distance, bool reversed){
    mRead = new Read(*r);
    mDistance = distance;
    mPos = pos;
    mReversed = reversed;
}

Match::~Match(){
    delete mRead;
    mRead = NULL;
    for(int i=0;i<mOriginalReads.size();i++){
        delete mOriginalReads[i];
        mOriginalReads[i] = NULL;
    }
}

void Match::addOriginalRead(Read* r){
    mOriginalReads.push_back(new Read(*r));
}

void Match::addOriginalPair(ReadPair* pair){
    mOriginalReads.push_back(new Read(*pair->mLeft));
    mOriginalReads.push_back(new Read(*pair->mRight));
}

void Match::print(int leftlen, int centerlen, int rightlen){
    cout<<"pos: "<<mPos<<", distance: "<<mDistance;
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
}

void Match::printHtmlTD(ofstream& file, int leftlen, int centerlen, int rightlen){
    file<<"<a title='"<<mRead->mName<<"'>";
    file<<"d:" << mDistance;
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
    mRead->printHtmlTDWithBreaks(file, breaks);
}

void Match::printReadsToFile(ofstream& file){
    for(int i=0;i<mOriginalReads.size();i++){
        mOriginalReads[i]->printFile(file);
    }
}

void Match::setReversed(bool flag){
    mReversed = flag;
}