#include "sescanner.h"
#include "scanner.h"

SingleEndScanner::SingleEndScanner(string mutationFile, string refFile, string read1File, string html, string json, int threadNum)
    : Scanner(mutationFile, refFile, read1File, "", html, json, threadNum) {
}

SingleEndScanner::~SingleEndScanner() {}

bool SingleEndScanner::scanNextEnd(ReadPack<Read>* pack){
    bool simplified = GlobalSettings::simplifiedMode;
    for(int p=0;p<pack->count;p++){
        Read* r1 = pack->data[p];
        Read* rcr1 = r1->reverseComplement();
        if(!scanRead(rcr1, r1, true) || simplified) delete rcr1;
        if(!scanRead(r1, r1, false) || simplified) delete r1;
    }

    delete pack->data;
    delete pack;

    return true;
}
