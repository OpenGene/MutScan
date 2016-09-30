#include "sescanner.h"
#include "fastqreader.h"
#include <iostream>
#include "htmlreporter.h"
#include <unistd.h>
#include <functional>
#include <thread>

SingleEndScanner::SingleEndScanner(string mutationFile, string read1File, string read2File, string html){
    mRead1File = read1File;
    mRead2File = read2File;
    mMutationFile = mutationFile;
    mHtmlFile = html;
    mProduceFinished = false;
}

bool SingleEndScanner::scan(){

    if(mMutationFile!="")
        mutationList = Mutation::parseFile(mMutationFile);
    else
        mutationList = Mutation::parseBuiltIn();

    mutationMatches = new vector<Match*>[mutationList.size()];
    for(int i=0;i<mutationList.size();i++){
        mutationMatches[i] = vector<Match*>();
    }

    initReadRepository();
    std::thread producer(std::bind(&SingleEndScanner::producerTask, this));
    std::thread consumer1(std::bind(&SingleEndScanner::consumerTask, this));
    //std::thread consumer2(std::bind(&SingleEndScanner::consumerTask, this));
    //std::thread consumer3(std::bind(&SingleEndScanner::consumerTask, this));
    //std::thread consumer4(std::bind(&SingleEndScanner::consumerTask, this));

    producer.join();
    consumer1.join();
    //consumer2.join();
    //consumer3.join();
    //consumer4.join();

    textReport(mutationList, mutationMatches);
    htmlReport(mutationList, mutationMatches);

    // free memory
    for(int i=0;i<mutationList.size();i++){
        mutationMatches[i].clear();
    }
}

void SingleEndScanner::pushMatch(int i, Match* m){
    std::unique_lock<std::mutex> lock(mMutationMtx);
    cout<<"add"<<endl;
    mutationMatches[i].push_back(m);
    lock.unlock();
}

bool SingleEndScanner::scanSingleEnd(ReadPack* pack){
    for(int p=0;p<pack->count;p++){
        Read* r1 = pack->data[p];
        Read* rcr1 = r1->reverseComplement();
        for(int i=0;i<mutationList.size();i++){
            Match* matchR1 = mutationList[i].searchInRead(r1);
            if(matchR1)
                pushMatch(i, matchR1);
            Match* matchRcr1 = mutationList[i].searchInRead(rcr1);
            if(matchRcr1){
                matchRcr1->setReversed(true);
                pushMatch(i, matchRcr1);
            }
        }
        //delete r1;
        //delete rcr1;
    }

    //delete pack->data;
    //delete pack;

    return true;
}

void SingleEndScanner::initReadRepository() {
    mRepo.read_buffer = new ReadPack*[kReadRepositorySize];
    memset(mRepo.read_buffer, 0, sizeof(ReadPack*)*kReadRepositorySize);
    mRepo.write_position = 0;
    mRepo.read_position = 0;
    mRepo.read_counter = 0;
    
}

void SingleEndScanner::destroyReadRepository() {
    delete mRepo.read_buffer;
    mRepo.read_buffer = NULL;
}

void SingleEndScanner::produceRead(ReadPack* pack){
    std::unique_lock<std::mutex> lock(mRepo.mtx);
    /*while(((mRepo.write_position + 1) % kReadRepositorySize)
        == mRepo.read_position) {
        (mRepo.repo_not_full).wait(lock);
    }*/

    (mRepo.read_buffer)[mRepo.write_position] = pack;
    (mRepo.write_position)++;

    if (mRepo.write_position == kReadRepositorySize)
        mRepo.write_position = 0;

    (mRepo.repo_not_empty).notify_all();
    lock.unlock();
}

void SingleEndScanner::consumeRead(){
    ReadPack* data;
    std::unique_lock<std::mutex> lock(mRepo.mtx);
    // read buffer is empty, just wait here.
    while(mRepo.write_position == mRepo.read_position) {
        (mRepo.repo_not_empty).wait(lock);
    }

    data = (mRepo.read_buffer)[mRepo.read_position];
    (mRepo.read_position)++;
    lock.unlock();

    scanSingleEnd(data);

    if (mRepo.read_position >= kReadRepositorySize)
        mRepo.read_position = 0;

    //(mRepo.repo_not_full).notify_all();
}

void SingleEndScanner::producerTask()
{
    const int packsize = 1000;
    Read** data = new Read*[packsize];
    memset(data, 0, sizeof(Read*)*packsize);
    FastqReader reader1(mRead1File);
    int count=0;
    while(true){
        Read* read = reader1.read();
        if(!read){
            // the last pack
            if(count>0){
                ReadPack* pack = new ReadPack;
                pack->data = data;
                pack->count = count;
                produceRead(pack);
            }
            break;
        }
        data[count] = read;
        count++;
        // a full pack
        if(count == packsize){
            ReadPack* pack = new ReadPack;
            pack->data = data;
            pack->count = count;
            produceRead(pack);
            // reset count to 0
            count = 0;
        }
    }
    mProduceFinished = true;
}

void SingleEndScanner::consumerTask()
{
    bool ready_to_exit = false;
    while(1) {
        std::unique_lock<std::mutex> lock(mRepo.read_counter_mtx);
        if(mProduceFinished && mRepo.write_position == mRepo.read_position){
            lock.unlock();
            break;
        }

        lock.unlock();

        consumeRead();
        ++(mRepo.read_counter);

    }
}

void SingleEndScanner::textReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches) {
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

void SingleEndScanner::htmlReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches) {
    if(mHtmlFile == "")
        return;

    HtmlReporter reporter(mHtmlFile, mutationList, mutationMatches);
    reporter.run();
}
