#ifndef MUT_SCAN_H
#define MUT_SCAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "read.h"
#include "mutation.h"

using namespace std;

class MutScan{
public:
    MutScan(string read1File, string read2File);
    bool scan();

private:
    string mRead1File;
    string mRead2File;
};


#endif