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

static const int kReadRepositorySize  = 100000000;

struct ReadRepository {
    Read** read_buffer;
    size_t read_position;
    size_t write_position;
    size_t read_counter;
    std::mutex mtx;
    std::mutex read_counter_mtx;
    std::condition_variable repo_not_full;
    std::condition_variable repo_not_empty;
};

typedef struct ReadRepository ReadRepository;

class SingleEndScanner{
public:
    SingleEndScanner(string mutationFile, string read1File, string read2File, string html);
    bool scan();
    void textReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches);
    void htmlReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches);

private:
    bool scanSingleEnd(Read* r1);
    void initReadRepository();
    void destroyReadRepository();
    void produceRead(Read* read);
    Read* consumeRead();
    void producerTask();
    void consumerTask();

private:
    string mMutationFile;
    string mRead1File;
    string mRead2File;
    string mHtmlFile;
    ReadRepository mRepo;
    bool mProduceFinished;
    vector<Mutation> mutationList;
    vector<Match*> *mutationMatches;
};


#endif