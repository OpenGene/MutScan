#ifndef SCANNNER_IMPL_H
#define SCANNNER_IMPL_H

#include "scanner.h"
#include "fastqreader.h"
#include <iostream>
#include "htmlreporter.h"
#include "jsonreporter.h"
#include "multihtmlreporter.h"
#include <unistd.h>
#include <functional>
#include <thread>
#include <memory.h>
#include "util.h"
#include "globalsettings.h"
#include "mutscan.h"

template <class ReadData, class Self>
Scanner<ReadData, Self>::Scanner(string mutationFile, string refFile, string read1File, string read2File, string html, string json, int threadNum){
    mMutationFile = mutationFile;
    mRefFile = refFile;
    mRead1File = read1File;
    mRead2File = read2File;
    mHtmlFile = html;
    mJsonFile = json;
    mProduceFinished = false;
    mThreadNum = threadNum;
    mRollingHash = NULL;
}

template <class ReadData, class Self>
Scanner<ReadData, Self>::~Scanner() {
    if(mRollingHash){
        delete mRollingHash;
        mRollingHash = NULL;
    }
    for(size_t i=0; i<mReadToDelete.size(); i++) {
        delete mReadToDelete[i];
        mReadToDelete[i] = NULL;
    }
    for(size_t i=0; i<mBufToDelete.size(); i++) {
        delete mBufToDelete[i];
        mBufToDelete[i] = NULL;
    }
}


template <class ReadData, class Self>
bool Scanner<ReadData, Self>::scan(){

    if(mMutationFile!=""){
        if(ends_with(mMutationFile, ".vcf") || ends_with(mMutationFile, ".VCF") || ends_with(mMutationFile, ".Vcf"))
            mutationList = Mutation::parseVcf(mMutationFile, mRefFile);
        else
            mutationList = Mutation::parseCsv(mMutationFile);
    }
    else
        mutationList = Mutation::parseBuiltIn();

    if(GlobalSettings::verbose)
        cerr << "Scanning "<< mutationList.size() << " mutations..."<<endl;

    if(GlobalSettings::simplifiedModeToEvaluate)
        MutScan::evaluateSimplifiedMode(mRead1File, mRead2File, mutationList.size());

    if(!GlobalSettings::legacyMode && mRollingHash==NULL){
        mRollingHash = new RollingHash();
        mRollingHash->initMutations(mutationList);
    }

    mutationMatches = new vector<Match*>[mutationList.size()];
    for(size_t i=0;i<mutationList.size();i++){
        mutationMatches[i] = vector<Match*>();
    }

    initPackRepository();
    std::thread producer(std::bind(&Self::producerTask, this));

    std::thread** threads = new thread*[mThreadNum];
    for(int t=0; t<mThreadNum; t++){
        threads[t] = new std::thread(std::bind(&Self::consumerTask, this));
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
    for(size_t i=0;i<mutationList.size();i++){
        sort(mutationMatches[i].begin(), mutationMatches[i].end(), Match::greater); 
    }

    textReport(mutationList, mutationMatches);
    htmlReport(mutationList, mutationMatches);
    jsonReport(mutationList, mutationMatches);

    // free memory
    for(size_t i=0;i<mutationList.size();i++){
        mutationMatches[i].clear();
    }
    return true;
}

template <class ReadData, class Self>
void Scanner<ReadData, Self>::pushMatch(int i, Match* m, bool needStoreReadToDelete){
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

template <class ReadData, class Self>
bool Scanner<ReadData, Self>::scanRead(Read* r, ReadData* originalRead, bool reversed) {
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
                    match->addOriginalReadData(originalRead);
                match->setReversed(reversed);
                pushMatch(t, match, !matched);
                matched = true;
            }
        }
    } else {
        for(size_t i=0;i<mutationList.size();i++){
            Match* match = mutationList[i].searchInRead(r, simplifiedBuf);
            if(match) {
                if(!GlobalSettings::simplifiedMode)
                    match->addOriginalReadData(originalRead);
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

template <class ReadData, class Self>
void Scanner<ReadData, Self>::initPackRepository() {
    mRepo.packBuffer = new ReadPack<ReadData>*[PACK_NUM_LIMIT];
    memset(mRepo.packBuffer, 0, sizeof(ReadPack<ReadData>*)*PACK_NUM_LIMIT);
    mRepo.writePos = 0;
    mRepo.readPos = 0;
    mRepo.readCounter = 0;
    
}

template <class ReadData, class Self>
void Scanner<ReadData, Self>::destroyPackRepository() {
    delete mRepo.packBuffer;
    mRepo.packBuffer = NULL;
}

template <class ReadData, class Self>
void Scanner<ReadData, Self>::producePack(ReadPack<ReadData>* pack){
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

template <class ReadData, class Self>
void Scanner<ReadData, Self>::consumePack(){
    ReadPack<ReadData>* data;
    std::unique_lock<std::mutex> lock(mRepo.mtx);
    // read buffer is empty, just wait here.
    while(mRepo.writePos % PACK_NUM_LIMIT == mRepo.readPos % PACK_NUM_LIMIT) {
        if(mProduceFinished){
            lock.unlock();
            return;
        }
        mRepo.repoNotEmpty.wait(lock);
    }

    data = mRepo.packBuffer[mRepo.readPos];
    mRepo.readPos++;

    if (mRepo.readPos >= PACK_NUM_LIMIT)
        mRepo.readPos = 0;

    lock.unlock();
    mRepo.repoNotFull.notify_all();

    static_cast<Self*>(this)->scanNextEnd(data);
}

template <class ReadData, class Self>
void Scanner<ReadData, Self>::producerTask()
{
    int slept = 0;
    long total = 0;
    ReadData** data = new ReadData*[PACK_SIZE];
    memset(data, 0, sizeof(ReadData*)*PACK_SIZE);
    typename reader_trait<ReadData>::FastqReaderType *reader = static_cast<Self*>(this)->fastqReader();
    int count=0;
    while(true){
        ReadData* read = reader->read();
        if(!read){
            // the last pack
            ReadPack<ReadData>* pack = new ReadPack<ReadData>;
            pack->data = data;
            pack->count = count;
            producePack(pack);
            data = NULL;
            if(GlobalSettings::verbose) {
                cerr << "Loaded all of " << total << " " << reader_trait<ReadData>::name << endl;
            }
            break;
        }
        data[count] = read;
        count++;
        total++;
        // a full pack
        if(count == PACK_SIZE){
            ReadPack<ReadData>* pack = new ReadPack<ReadData>;
            pack->data = data;
            pack->count = count;
            producePack(pack);
            //re-initialize data for next pack
            data = new ReadData*[PACK_SIZE];
            memset(data, 0, sizeof(ReadData*)*PACK_SIZE);
            // reset count to 0
            count = 0;
            // if the consumer is far behind this producer, sleep and wait to limit memory usage
            while(mRepo.writePos - mRepo.readPos > PACK_IN_MEM_LIMIT){
                //cout<<"sleep"<<endl;
                slept++;
                usleep(1000);
            }
            if(GlobalSettings::verbose && total % 1000000 == 0) {
                cerr << "Loaded " << total << " " << reader_trait<ReadData>::name << endl;
            }
        }
    }

    std::unique_lock<std::mutex> lock(mRepo.mtx);
    mProduceFinished = true;
    // ensure all waiting consommers exit
    mRepo.repoNotEmpty.notify_all();
    lock.unlock();

    // if the last data initialized is not used, free it
    if(data != NULL)
        delete data;
    delete reader;
}

template <class ReadData, class Self>
void Scanner<ReadData, Self>::consumerTask()
{
    while(true) {
        std::unique_lock<std::mutex> lock(mRepo.mtx);
        if(mProduceFinished && mRepo.writePos == mRepo.readPos){
            lock.unlock();
            break;
        }
        lock.unlock();
        consumePack();
    }
}

template <class ReadData, class Self>
void Scanner<ReadData, Self>::textReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches) {
    //output result
    bool found = false;
    int undetected = 0;
    for(size_t i=0;i<mutationList.size();i++){
        vector<Match*> matches = mutationMatches[i];
        if((ssize_t)matches.size()>=GlobalSettings::minReadSupport){
            found = true;
            cout<<endl<<"---------------"<<endl;
            mutationList[i].print();
            for(size_t m=0; m<matches.size(); m++){
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
        for(size_t i=0;i<mutationList.size();i++){
            vector<Match*> matches = mutationMatches[i];
            if((ssize_t)matches.size()<GlobalSettings::minReadSupport){
                Mutation m = mutationList[i];
                cerr <<m.mChr << " " <<m.mName<<" "<<m.mLeft<<" "<<m.mCenter<<" "<<m.mRight <<endl;
            }
        }
    }
}

template <class ReadData, class Self>
void Scanner<ReadData, Self>::jsonReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches) {
    if(mJsonFile == "")
        return;

    JsonReporter reporter(mJsonFile, mutationList, mutationMatches);
    reporter.run();
}

template <class ReadData, class Self>
void Scanner<ReadData, Self>::htmlReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches) {
    if(mHtmlFile == "")
        return;

    // display a warning for too many reads with standalone mode
    if(GlobalSettings::standaloneHtml){
        int totalReadSize = 0;
        for(size_t i=0;i<mutationList.size();i++){
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

#endif
