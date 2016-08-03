#include <stdio.h>
#include "fastqreader.h"
#include "unittest.h"
#include "mutscan.h"

int main(int argc, char* argv[]){
    if (argc == 2 && strcmp(argv[1], "test")==0){
        UnitTest tester;
        tester.run();
        return 0;
    }
	if(argc<4){
		printf("Usage:\n\tmutscan <mutation_file> <read1_file> <read2_file>\n");
        printf("Testing:\n\tmutscan test\n");
		exit(1);
	}
    string mutationFile(argv[1]);
    string r1file(argv[2]);
    string r2file(argv[3]);
    MutScan scanner(mutationFile, r1file, r2file);
    scanner.scan();
}