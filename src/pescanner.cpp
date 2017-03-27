#include "pescanner.h"
#include "fastqreader.h"
#include <iostream>
#include "htmlreporter.h"
#include "multihtmlreporter.h"
#include <unistd.h>
#include <functional>
#include <thread>
#include <memory.h>
#include "util.h"
#include "globalsettings.h"

PairEndScanner::PairEndScanner(string mutationFile, string refFile, string read1File, string read2File, string html, int threadNum){
    mRead1File = read1File;
    mRead2File = read2File;
    mMutationFile = mutationFile;
    mRefFile = refFile;
    mHtmlFile = html;
    mProduceFinished = false;
    mThreadNum = threadNum;
    mRollingHash = NULL;
}

PairEndScanner::~PairEndScanner() {
    if(mRollingHash){
        delete mRollingHash;
        mRollingHash = NULL;
    }
}


bool PairEndScanner::scan(){

    if(mMutationFile!=""){
        if(ends_with(mMutationFile, ".vcf") || ends_with(mMutationFile, ".VCF") || ends_with(mMutationFile, ".Vcf"))
            mutationList = Mutation::parseVcf(mMutationFile, mRefFile);
        else
            mutationList = Mutation::parseCsv(mMutationFile);
    }
    else
        mutationList = Mutation::parseBuiltIn();

    if(!GlobalSettings::legacyMode && mRollingHash==NULL){
        mRollingHash = new RollingHash();
        mRollingHash->initMutations(mutationList);
    }

    mutationMatches = new vector<Match*>[mutationList.size()];
    for(int i=0;i<mutationList.size();i++){
        mutationMatches[i] = vector<Match*>();
    }

    initPackRepository();
    std::thread producer(std::bind(&PairEndScanner::producerTask, this));

    std::thread** threads = new thread*[mThreadNum];
    for(int t=0; t<mThreadNum; t++){
        threads[t] = new std::thread(std::bind(&PairEndScanner::consumerTask, this));
    }

    producer.join();
    for(int t=0; t<mThreadNum; t++){
        threads[t]->join();
    }

    for(int t=0; t<mThreadNum; t++){
        delete threads[t];
        threads[t] = NULL;
    }

    // sort the matches to make the pileup more clear
    for(int i=0;i<mutationList.size();i++){
        sort(mutationMatches[i].begin(), mutationMatches[i].end(), Match::greater); 
    }

    textReport(mutationList, mutationMatches);
    htmlReport(mutationList, mutationMatches);

    // free memory
    for(int i=0;i<mutationList.size();i++){
        mutationMatches[i].clear();
    }
}

void PairEndScanner::pushMatch(int i, Match* m){
    std::unique_lock<std::mutex> lock(mMutationMtx);
    mutationMatches[i].push_back(m);
    lock.unlock();
}

bool PairEndScanner::scanPairEnd(ReadPairPack* pack){
    for(int p=0;p<pack->count;p++){
        ReadPair* pair = pack->data[p];
        Read* r1 = pair->mLeft;
        Read* r2 = pair->mRight;
        Read* rcr1 = NULL;
        Read* rcr2 = NULL;
        Read* merged = pair->fastMerge();
        Read* mergedRC = NULL;
        if(merged != NULL)
            mergedRC = merged->reverseComplement();
        else {
            rcr1 = r1->reverseComplement();
            rcr2 = r2->reverseComplement();
        }

        if(merged != NULL) {
            scanRead(merged, pair, false);
            scanRead(mergedRC, pair, true);
        } else {
            scanRead(r1, pair, false);
            scanRead(r2, pair, false);
            scanRead(rcr1, pair, true);
            scanRead(rcr2, pair, true);
        }

        delete pair;
        if(merged!=NULL){
            delete merged;
            delete mergedRC;
        } else {
            delete rcr1;
            delete rcr2;
        }
    }

    delete pack->data;
    delete pack;

    return true;
}

bool PairEndScanner::scanRead(Read* r, ReadPair* originalPair, bool reversed) {
    if(!GlobalSettings::legacyMode){
        vector<int> targets = mRollingHash->hitTargets(r->mSeq.mStr);
        for(int t=0; t<targets.size(); t++) {
            Match* match = mutationList[targets[t]].searchInRead(r);
            if(match) {
                if(GlobalSettings::outputOriginalReads)
                    match->addOriginalPair(originalPair);
                match->setReversed(reversed);
                pushMatch(targets[t], match);
            }
        }
    } else {
        for(int i=0;i<mutationList.size();i++){
            Match* match = mutationList[i].searchInRead(r);
            if(match) {
                if(GlobalSettings::outputOriginalReads)
                    match->addOriginalPair(originalPair);
                match->setReversed(reversed);
                pushMatch(i, match);
            }
        }
    }
}

void PairEndScanner::initPackRepository() {
    mRepo.packBuffer = new ReadPairPack*[PACK_NUM_LIMIT];
    memset(mRepo.packBuffer, 0, sizeof(ReadPairPack*)*PACK_NUM_LIMIT);
    mRepo.writePos = 0;
    mRepo.readPos = 0;
    mRepo.readCounter = 0;
    
}

void PairEndScanner::destroyPackRepository() {
    delete mRepo.packBuffer;
    mRepo.packBuffer = NULL;
}

void PairEndScanner::producePack(ReadPairPack* pack){
    std::unique_lock<std::mutex> lock(mRepo.mtx);
    while(((mRepo.writePos + 1) % PACK_NUM_LIMIT)
        == mRepo.readPos) {
        mRepo.repoNotFull.wait(lock);
    }

    mRepo.packBuffer[mRepo.writePos] = pack;
    mRepo.writePos++;

    if (mRepo.writePos == PACK_NUM_LIMIT)
        mRepo.writePos = 0;

    mRepo.repoNotEmpty.notify_all();
    lock.unlock();
}

void PairEndScanner::consumePack(){
    ReadPairPack* data;
    std::unique_lock<std::mutex> lock(mRepo.mtx);
    // read buffer is empty, just wait here.
    while(mRepo.writePos == mRepo.readPos) {
        if(mProduceFinished){
            lock.unlock();
            return;
        }
        mRepo.repoNotEmpty.wait(lock);
    }

    data = mRepo.packBuffer[mRepo.readPos];
    (mRepo.readPos)++;
    lock.unlock();

    scanPairEnd(data);


    if (mRepo.readPos >= PACK_NUM_LIMIT)
        mRepo.readPos = 0;

    mRepo.repoNotFull.notify_all();
}

void PairEndScanner::producerTask()
{
    int slept = 0;
    ReadPair** data = new ReadPair*[PACK_SIZE];
    memset(data, 0, sizeof(ReadPair*)*PACK_SIZE);
    FastqReaderPair reader(mRead1File, mRead2File);
    int count=0;
    while(true){
        ReadPair* read = reader.read();
        if(!read){
            // the last pack
            if(count>0){
                ReadPairPack* pack = new ReadPairPack;
                pack->data = data;
                pack->count = count;
                producePack(pack);
            }
            data = NULL;
            break;
        }
        data[count] = read;
        count++;
        // a full pack
        if(count == PACK_SIZE){
            ReadPairPack* pack = new ReadPairPack;
            pack->data = data;
            pack->count = count;
            producePack(pack);
            //re-initialize data for next pack
            data = new ReadPair*[PACK_SIZE];
            memset(data, 0, sizeof(ReadPair*)*PACK_SIZE);
            // reset count to 0
            count = 0;
            // if the consumer is far behind this producer, sleep and wait to limit memory usage
            while(mRepo.writePos - mRepo.readPos > PACK_IN_MEM_LIMIT){
                //cout<<"sleep"<<endl;
                slept++;
                usleep(1000);
            }
        }
    }

    std::unique_lock<std::mutex> lock(mRepo.readCounterMtx);
    mProduceFinished = true;
    lock.unlock();

    // if the last data initialized is not used, free it
    if(data != NULL)
        delete data;
}

void PairEndScanner::consumerTask()
{
    while(true) {
        std::unique_lock<std::mutex> lock(mRepo.readCounterMtx);
        if(mProduceFinished && mRepo.writePos == mRepo.readPos){
            lock.unlock();
            break;
        }
        if(mProduceFinished){
            consumePack();
            lock.unlock();
        } else {
            lock.unlock();
            consumePack();
        }
    }
}

void PairEndScanner::textReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches) {
    //output result
    for(int i=0;i<mutationList.size();i++){
        vector<Match*> matches = mutationMatches[i];
        if(matches.size()>0){
            cout<<endl<<"---------------"<<endl;
            mutationList[i].print();
            for(int m=0; m<matches.size(); m++){
                cout<<m+1<<", ";
                matches[m]->print(mutationList[i].mLeft.length(), mutationList[i].mCenter.length(), mutationList[i].mRight.length());
            }
        }
    }
}

void PairEndScanner::htmlReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches) {
    if(mHtmlFile == "")
        return;

    // display a warning for too many reads with standalone mode
    if(GlobalSettings::standaloneHtml){
        int totalReadSize = 0;
        for(int i=0;i<mutationList.size();i++){
            vector<Match*> matches = mutationMatches[i];
            totalReadSize += matches.size();
        }
        if(totalReadSize > WARN_STANDALONE_READ_LIMIT)
            cerr << "WARNING: found too many (" << totalReadSize << ") reads. The standalone HTML report file will be very big and not readable. Remove -s or --standalone to generate multiple HTML files for each mutation."<<endl;
    }

    if(!GlobalSettings::standaloneHtml) {
        MultiHtmlReporter reporter(mHtmlFile, mutationList, mutationMatches);
        reporter.run();
    } else {
        HtmlReporter reporter(mHtmlFile, mutationList, mutationMatches);
        reporter.run();
    }
}
