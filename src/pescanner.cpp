#include "pescanner.h"

PairEndScanner::PairEndScanner(string mutationFile, string refFile, string read1File, string read2File, string html, string json, int threadNum)
    : Scanner(mutationFile, refFile, read1File, read2File, html, json, threadNum) {
}

PairEndScanner::~PairEndScanner() {}

bool PairEndScanner::scanNextEnd(ReadPack<ReadPair>* pack){
    bool simplified = GlobalSettings::simplifiedMode;

    for(int p=0;p<pack->count;p++){
        ReadPair* pair = pack->data[p];
        Read* r1 = pair->mLeft;
        Read* r2 = pair->mRight;
        Read* rcr1 = NULL;
        Read* rcr2 = NULL;
        Read* merged = pair->fastMerge();
        Read* mergedRC = NULL;
        if(merged != NULL)
            mergedRC = merged->reverseComplement();
        else {
            rcr1 = r1->reverseComplement();
            rcr2 = r2->reverseComplement();
        }

        if(merged != NULL) {
            if(!scanRead(merged, pair, false) || simplified) delete merged;
            if(!scanRead(mergedRC, pair, true) || simplified) delete mergedRC;
        } else {
            if(!scanRead(rcr1, pair, true) || simplified) delete rcr1;
            if(!scanRead(rcr2, pair, true) || simplified) delete rcr2;
            bool leftMatched = scanRead(r1, pair, false);
            bool rightMatched = scanRead(r2, pair, false);
            if(leftMatched && !simplified) {
                pair->mLeft = NULL;
            }
            if(rightMatched && !simplified) {
                pair->mRight = NULL;
            }
        }

        delete pair;
    }

    delete pack->data;
    delete pack;

    return true;
}
