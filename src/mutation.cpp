#include "mutation.h"
#include "editdistance.h"

Mutation::Mutation(string left, string center, string right, string name){
	mLeft = left;
	mCenter = center;
    mRight = right;
    mPattern = left + center + right;
    mName = name;
}

int Mutation::searchInRead(Read* r, int distance){
    int readLen = r->mSeq.length();
    int lLen = mLeft.length();
    int cLen = mCenter.length();
    int rLen = mRight.length();
    int pLen = mPattern.length();
    string seq = r->mSeq.mStr;
    if(cLen > 0) {
        for(int start = lLen; start + cLen + rLen < readLen; start++){
            if(seq.substr(start, cLen) == mCenter){
                if (edit_distance(seq.substr(start - lLen, pLen), mPattern) <= distance){
                    cout<<"Mutation: "<<mName<<endl;
                    r->print();
                }
            }
        }
    }
}