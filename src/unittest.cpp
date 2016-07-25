#include "unittest.h"
#include "editdistance.h"
#include "sequence.h"
#include <time.h>

UnitTest::UnitTest(){

}

void UnitTest::run(){
    bool passed = true;
    passed &= editdistance_test();
    passed &= Sequence::test();
    printf("%s\n", passed?"PASSED":"FAILED");
}