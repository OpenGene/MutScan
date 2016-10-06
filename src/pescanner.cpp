#include "pescanner.h"
#include "fastqreader.h"
#include <iostream>
#include "htmlreporter.h"
#include <unistd.h>
#include <functional>
#include <thread>
#include <memory.h>

PairEndScanner::PairEndScanner(string mutationFile, string read1File, string read2File, string html, int threadNum){
    mRead1File = read1File;
    mRead2File = read2File;
    mMutationFile = mutationFile;
    mHtmlFile = html;
    mProduceFinished = false;
    mThreadNum = threadNum;
}

bool PairEndScanner::scan(){

    if(mMutationFile!="")
        mutationList = Mutation::parseFile(mMutationFile);
    else
        mutationList = Mutation::parseBuiltIn();

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
        Read* rcr1 = r1->reverseComplement();
        Read* rcr2 = r2->reverseComplement();
        for(int i=0;i<mutationList.size();i++){
            Match* matchR1 = mutationList[i].searchInRead(r1);
            if(matchR1){
                matchR1->addOriginalPair(pair);
                pushMatch(i, matchR1);
            }
            Match* matchR2 = mutationList[i].searchInRead(r2);
            if(matchR2){
                matchR2->addOriginalPair(pair);
                pushMatch(i, matchR2);
            }
            Match* matchRcr1 = mutationList[i].searchInRead(rcr1);
            if(matchRcr1){
                matchRcr1->addOriginalPair(pair);
                matchRcr1->setReversed(true);
                pushMatch(i, matchRcr1);
            }
            Match* matchRcr2 = mutationList[i].searchInRead(rcr2);
            if(matchRcr2){
                matchRcr2->addOriginalPair(pair);
                matchRcr2->setReversed(true);
                pushMatch(i, matchRcr2);
            }
        }
        delete pair;
        delete rcr1;
        delete rcr2;
    }

    delete pack->data;
    delete pack;

    return true;
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

    HtmlReporter reporter(mHtmlFile, mutationList, mutationMatches);
    reporter.run();
}
