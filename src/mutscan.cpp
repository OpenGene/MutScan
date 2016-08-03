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
    vector<Match*> *mutationMatches = new vector<Match*>[mutationList.size()];
    for(int i=0;i<mutationList.size();i++){
        mutationMatches[i] = vector<Match*>();
    }
    int processed = 0;
	while(true){
		Read* r1 = reader1.read();
		Read* r2 = reader2.read();
        if(!r1 || !r2)
            break;
        Read* rcr1 = r1->reverseComplement();
        Read* rcr2 = r2->reverseComplement();
        for(int i=0;i<mutationList.size();i++){
	        Match* matchR1 = mutationList[i].searchInRead(r1);
            if(matchR1)
                mutationMatches[i].push_back(matchR1);
	        Match* matchR2 = mutationList[i].searchInRead(r2);
            if(matchR2)
                mutationMatches[i].push_back(matchR2);
            Match* matchRcr1 = mutationList[i].searchInRead(rcr1);
            if(matchRcr1){
                matchRcr1->setReversed(true);
                mutationMatches[i].push_back(matchRcr1);
            }
            Match* matchRcr2 = mutationList[i].searchInRead(rcr2);
            if(matchRcr2){
                matchRcr2->setReversed(true);
                mutationMatches[i].push_back(matchRcr2);
            }
	    }
        delete r1;
        delete r2;
        delete rcr1;
        delete rcr2;

        processed += 1;
        if(processed % 1000000 == 0) {
            cout<<"processed "<<processed<<" reads"<<endl;
        }
	}

    //output result
    for(int i=0;i<mutationList.size();i++){
        vector<Match*> matches = mutationMatches[i];
        if(matches.size()>0){
            cout<<endl<<"---------------"<<endl;
            mutationList[i].print();
            for(int m=0; m<matches.size(); m++){
                cout<<m+1<<", ";
                matches[m]->print();
            }
        }
    }
	return true;
}