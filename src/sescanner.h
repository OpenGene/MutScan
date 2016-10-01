#ifndef SE_SCANNNER_H
#define SE_SCANNNER_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "read.h"
#include "mutation.h"
#include <cstdlib>
#include <condition_variable>
#include <mutex>
#include <thread>


using namespace std;

struct ReadPack {
    Read** data;
    int count;
};

typedef struct ReadPack ReadPack;

struct ReadRepository {
    ReadPack** packBuffer;
    size_t readPos;
    size_t writePos;
    size_t readCounter;
    std::mutex mtx;
    std::mutex readCounterMtx;
    std::condition_variable repoNotFull;
    std::condition_variable repoNotEmpty;
};

typedef struct ReadRepository ReadRepository;

class SingleEndScanner{
public:
    SingleEndScanner(string mutationFile, string read1File, string html="", int threadnum=1);
    bool scan();
    void textReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches);
    void htmlReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches);

private:
    bool scanSingleEnd(ReadPack* pack);
    void initPackRepository();
    void destroyPackRepository();
    void producePack(ReadPack* pack);
    void consumePack();
    void producerTask();
    void consumerTask();
    void pushMatch(int i, Match* m);

private:
    string mMutationFile;
    string mRead1File;
    string mRead2File;
    string mHtmlFile;
    ReadRepository mRepo;
    bool mProduceFinished;
    vector<Mutation> mutationList;
    vector<Match*> *mutationMatches;
    std::mutex mMutationMtx;
    int mThreadNum;
};


#endif