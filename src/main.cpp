#include <stdio.h>
#include "fastqreader.h"
#include "unittest.h"

int main(int argc, char* argv[]){
    if (argc == 2 && strcmp(argv[1], "test")==0){
        UnitTest tester;
        tester.run();
        return 0;
    }
	if(argc<3){
		printf("Usage:\n\tmutscan <read1_file> <read2_file>\n");
        printf("Testing:\n\tmutscan test\n");
		exit(1);
	}
}