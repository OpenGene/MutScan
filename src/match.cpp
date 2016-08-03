#include "match.h"

Match::Match(Read* r, int pos, int distance, bool reversed){
    mRead = new Read(*r);
    mDistance = distance;
    mPos = pos;
    mReversed = reversed;
}

Match::~Match(){
    delete mRead;
}

void Match::print(){
    cout<<"pos: "<<mPos<<", distance: "<<mDistance;
    if(mReversed)
        cout<<" forward";
    else
        cout<<" reversed";
    cout<<endl;
    mRead->print();
}

void Match::setReversed(bool flag){
    mReversed = flag;
}