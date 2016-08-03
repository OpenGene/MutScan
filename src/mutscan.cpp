#include "mutscan.h"
#include "fastqreader.h"

MutScan::MutScan(string mutationFile, string read1File, string read2File){
	mRead1File = read1File;
	mRead2File = read2File;
	mMutationFile = mutationFile;
}

bool MutScan::scan(){
	FastqReader reader1(mRead1File);
	FastqReader reader2(mRead2File);
	vector<Mutation> mutationList = Mutation::parseFile(mMutationFile);
	while(true){
		Read* r1 = reader1.read();
		Read* r2 = reader2.read();
        if(!r1 || !r2)
            break;
        for(int i=0;i<mutationList.size();i++){
        	Mutation mutation = mutationList[i];
	        mutation.searchInRead(r1);
	        mutation.searchInRead(r2);
	    }
	}
	return true;
}