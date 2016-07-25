#include "sequence.h"

Sequence::Sequence(string seq){
    mSeq = seq;
}

void Sequence::print(){
    std::cout << mSeq;
}

int Sequence::length(){
    return mSeq.length();
}

Sequence Sequence::reverseComplement(){
    string str(mSeq.length(), 0);
    for(int c=0;c<mSeq.length();c++){
        char base = mSeq[c];
        switch(base){
            case 'A':
            case 'a':
                str[mSeq.length()-c-1] = 'T';
                break;
            case 'T':
            case 't':
                str[mSeq.length()-c-1] = 'A';
                break;
            case 'C':
            case 'c':
                str[mSeq.length()-c-1] = 'G';
                break;
            case 'G':
            case 'g':
                str[mSeq.length()-c-1] = 'C';
                break;
            default:
                str[mSeq.length()-c-1] = 'N';
        }
    }
    return Sequence(str);
}

Sequence Sequence::operator~(){
    return reverseComplement();
}

bool Sequence::test(){
    Sequence s("AAAATTTTCCCCGGGG");
    Sequence rc = ~s;
    if (s.mSeq != "AAAATTTTCCCCGGGG" ){
        cout << "Failed in reverseComplement() expect AAAATTTTCCCCGGGG, but get "<< s.mSeq;
        return false;
    }
    if (rc.mSeq != "CCCCGGGGAAAATTTT" ){
        cout << "Failed in reverseComplement() expect CCCCGGGGAAAATTTT, but get "<< rc.mSeq;
        return false;
    }
    return true;
}