#include <stdio.h>
#include "fastqreader.h"
#include "unittest.h"
#include "mutscan.h"
#include <time.h>
#include "cmdline.h"

int main(int argc, char* argv[]){
    if (argc == 2 && strcmp(argv[1], "test")==0){
        UnitTest tester;
        tester.run();
        return 0;
    }
    cmdline::parser cmd;
    cmd.add<string>("read1", '1', "read1 file name", true, "");
    cmd.add<string>("read2", '2', "read2 file name", false, "");
    cmd.add<string>("mutation", 'm', "mutation file name", false, "");
    cmd.add<string>("html", 'h', "filename of html report, no html report if not specified", false, "");
    cmd.add<int>("thread", 't', "worker thread number, default is 4", false, 4);
    cmd.parse_check(argc, argv);
    string r1file = cmd.get<string>("read1");
    string r2file = cmd.get<string>("read2");
    string mutationFile = cmd.get<string>("mutation");
    string html = cmd.get<string>("html");
    int threadNum = cmd.get<int>("thread");

    clock_t t1 = clock();
    MutScan scanner(mutationFile, r1file, r2file, html, threadNum);
    scanner.scan();
    clock_t t2 = clock();
    printf("\nTime used: %f ms\n", (t2-t1)/1000.0);
}