#include <stdio.h>
#include "zlib/zlib.h"
#include "fastqreader.h"

int main(int argc, char* argv[]){
	if(argc<3){
		printf("usage:\n\tmutscan <read1_file> <read2_file>\n");
		exit(1);
	}
}