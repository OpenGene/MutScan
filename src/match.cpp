#include "match.h"

Match::Match(Read* r1, Read* r2, int distance, int pos, int type, bool reversed){
	if(r1){
        mRead1 = new Read(*r1);
    } else
        mRead1 = NULL;

    if(r2){
        mRead2 = new Read(*r2);
    } else
        mRead2 = NULL;

    mDistance = distance;
    mPos = pos;
    mType = type;
    mReversed = reversed;
}

Match::~Match(){
	if(mRead1){
        delete mRead1;
    }
    if(mRead2){
        delete mRead2;
    }
}