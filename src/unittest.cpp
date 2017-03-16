#include "unittest.h"
#include "editdistance.h"
#include "sequence.h"
#include "fastqreader.h"
#include "fastareader.h"
#include "vcfreader.h"
#include "overlap.h"
#include "read.h"
#include "rollinghash.h"
#include <time.h>

UnitTest::UnitTest(){

}

void UnitTest::run(){
    bool passed = true;
    passed &= editdistance_test();
    passed &= Sequence::test();
    passed &= FastqReader::test();
    passed &= FastaReader::test();
    passed &= VcfReader::test();
    passed &= Overlap::test();
    passed &= Read::test();
    passed &= ReadPair::test();
    passed &= RollingHash::test();
    printf("\n==========================\n");
    printf("%s\n\n", passed?"PASSED":"FAILED");
}