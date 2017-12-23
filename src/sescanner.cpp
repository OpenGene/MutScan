#include "sescanner.h"
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

SingleEndScanner::SingleEndScanner(string mutationFile, string refFile, string read1File, string html, int threadNum){
    mRead1File = read1File;
    mMutationFile = mutationFile;
    mRefFile = refFile;
    mHtmlFile = html;
    mProduceFinished = false;
    mThreadNum = threadNum;
    mRollingHash = NULL;
}

SingleEndScanner::~SingleEndScanner() {
    if(mRollingHash){
        delete mRollingHash;
        mRollingHash = NULL;
    }
    for(int i=0; i<mReadToDelete.size(); i++) {
        delete mReadToDelete[i];
        mReadToDelete[i] = NULL;
    }
    for(int i=0; i<mBufToDelete.size(); i++) {
        delete mBufToDelete[i];
        mBufToDelete[i] = NULL;
    }
}

bool SingleEndScanner::scan(){

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
    std::thread producer(std::bind(&SingleEndScanner::producerTask, this));

    std::thread** threads = new thread*[mThreadNum];
    for(int t=0; t<mThreadNum; t++){
        threads[t] = new std::thread(std::bind(&SingleEndScanner::consumerTask, this));
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
    return true;
}

void SingleEndScanner::pushMatch(int i, Match* m, bool needStoreReadToDelete){
    std::unique_lock<std::mutex> lock(mMutationMtx);
    mutationMatches[i].push_back(m);
    if(needStoreReadToDelete) {
        if(GlobalSettings::simplifiedMode)
            mBufToDelete.push_back(m->getSequence());
        else
            mReadToDelete.push_back(m->getRead());
    }
    lock.unlock();
}

bool SingleEndScanner::scanSingleEnd(ReadPack* pack){
    bool simplified = GlobalSettings::simplifiedMode;
    for(int p=0;p<pack->count;p++){
        Read* r1 = pack->data[p];
        Read* rcr1 = r1->reverseComplement();
        if(!scanRead(rcr1, r1, true) || simplified) delete rcr1;
        if(!scanRead(r1, r1, false) || simplified) delete r1;
    }

    delete pack->data;
    delete pack;

    return true;
}

bool SingleEndScanner::scanRead(Read* r, Read* originalRead, bool reversed) {
    // just copy the sequence buffer
    char* simplifiedBuf = NULL;
    if(GlobalSettings::simplifiedMode) {
        simplifiedBuf = r->to2bit();
        if(simplifiedBuf == NULL)
            return false;
    }
    bool matched = false;
    if(!GlobalSettings::legacyMode){
        map<int, int> targets = mRollingHash->hitTargets(r->mSeq.mStr);
        map<int, int>::iterator iter;
        for(iter=targets.begin(); iter!=targets.end(); iter++) {
            int t = iter->first;
            int count = iter->second;
            if(count==0)
                continue;
            Match* match = mutationList[t].searchInRead(r, simplifiedBuf);
            if(match) {
                if(!GlobalSettings::simplifiedMode)
                    match->addOriginalRead(originalRead);
                match->setReversed(reversed);
                pushMatch(t, match, !matched);
                matched = true;
            }
        }
    } else {
        for(int i=0;i<mutationList.size();i++){
            Match* match = mutationList[i].searchInRead(r, simplifiedBuf);
            if(match) {
                if(!GlobalSettings::simplifiedMode)
                    match->addOriginalRead(originalRead);
                match->setReversed(reversed);
                pushMatch(i, match, !matched);
                matched = true;
            }
        }
    }
    if(!matched && simplifiedBuf!=NULL) {
        delete simplifiedBuf;
        simplifiedBuf = NULL;
    }
    return matched;
}

void SingleEndScanner::initPackRepository() {
    mRepo.packBuffer = new ReadPack*[PACK_NUM_LIMIT];
    memset(mRepo.packBuffer, 0, sizeof(ReadPack*)*PACK_NUM_LIMIT);
    mRepo.writePos = 0;
    mRepo.readPos = 0;
    mRepo.readCounter = 0;
    
}

void SingleEndScanner::destroyPackRepository() {
    delete mRepo.packBuffer;
    mRepo.packBuffer = NULL;
}

void SingleEndScanner::producePack(ReadPack* pack){
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

void SingleEndScanner::consumePack(){
    ReadPack* data;
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

    scanSingleEnd(data);


    if (mRepo.readPos >= PACK_NUM_LIMIT)
        mRepo.readPos = 0;

    mRepo.repoNotFull.notify_all();
}

void SingleEndScanner::producerTask()
{
    int slept = 0;
    long total = 0;
    Read** data = new Read*[PACK_SIZE];
    memset(data, 0, sizeof(Read*)*PACK_SIZE);
    FastqReader reader1(mRead1File);
    int count=0;
    while(true){
        Read* read = reader1.read();
        if(!read){
            // the last pack
            ReadPack* pack = new ReadPack;
            pack->data = data;
            pack->count = count;
            producePack(pack);
            data = NULL;
            if(GlobalSettings::verbose) {
                cerr << "Loaded all of " << total << " reads" << endl;
            }
            break;
        }
        data[count] = read;
        count++;
        total++;
        // a full pack
        if(count == PACK_SIZE){
            ReadPack* pack = new ReadPack;
            pack->data = data;
            pack->count = count;
            producePack(pack);
            //re-initialize data for next pack
            data = new Read*[PACK_SIZE];
            memset(data, 0, sizeof(Read*)*PACK_SIZE);
            // reset count to 0
            count = 0;
            // if the consumer is far behind this producer, sleep and wait to limit memory usage
            while(mRepo.writePos - mRepo.readPos > PACK_IN_MEM_LIMIT){
                //cout<<"sleep"<<endl;
                slept++;
                usleep(1000);
            }
            if(GlobalSettings::verbose && total % 1000000 == 0) {
                cerr << "Loaded " << total << " reads" << endl;
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

void SingleEndScanner::consumerTask()
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

void SingleEndScanner::textReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches) {
    //output result
    bool found = false;
    int undetected = 0;
    for(int i=0;i<mutationList.size();i++){
        vector<Match*> matches = mutationMatches[i];
        if(matches.size()>=GlobalSettings::minReadSupport){
            found = true;
            cout<<endl<<"---------------"<<endl;
            mutationList[i].print();
            for(int m=0; m<matches.size(); m++){
                cout<<m+1<<", ";
                matches[m]->print(mutationList[i].mLeft.length(), mutationList[i].mCenter.length(), mutationList[i].mRight.length());
            }
        } else {
            undetected++;
        }
    }
    if(found == false) {
        cout << "MutScan didn't find any mutation" << endl;
    }
    // if processing VCF, output those with no supporting reads found
    if(GlobalSettings::processingVCF && undetected>0) {
        cerr << undetected << " mutations of this VCF are not detected" << endl;
        for(int i=0;i<mutationList.size();i++){
            vector<Match*> matches = mutationMatches[i];
            if(matches.size()<GlobalSettings::minReadSupport){
                Mutation m = mutationList[i];
                cerr <<m.mChr << " " <<m.mName<<" "<<m.mLeft<<" "<<m.mCenter<<" "<<m.mRight <<endl;
            }
        }
    }
}

void SingleEndScanner::htmlReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches) {
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
