#ifndef SCANNNER_H
#define SCANNNER_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "read.h"
#include "mutation.h"
#include <cstdlib>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "rollinghash.h"

using namespace std;

template <class ReadData>
class ReadPack {
public:
    ReadData** data;
    int count;
};

template <class ReadData>
class reader_trait {
public:
    //typedef unspecialized FastqReaderType;
    const char* name="unspecialized";
};

template <class ReadData>
class ReadRepository {
public:
    ReadPack<ReadData>** packBuffer;
    size_t readPos;
    size_t writePos;
    size_t readCounter;
    std::mutex mtx;
    std::condition_variable repoNotFull;
    std::condition_variable repoNotEmpty;
};

template <class ReadData, class Self>
class Scanner{
public:
    Scanner(string mutationFile, string refFile, string read1File, string read2File, string html="", string json = "", int threadnum=1);
    ~Scanner();
    bool scan();
    void textReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches);
    void htmlReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches);
    void jsonReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches);

protected:
    bool scanRead(Read* r, ReadData* originalRead, bool reversed);
private:
    void initPackRepository();
    void destroyPackRepository();
    void producePack(ReadPack<ReadData>* pack);
    void consumePack();
    void producerTask();
    void consumerTask();
    void pushMatch(int i, Match* m, bool needStoreReadToDelete);

private:
    string mMutationFile;
    string mRefFile;
protected:
    string mRead1File;
    string mRead2File;
private:
    string mHtmlFile;
    string mJsonFile;
    ReadRepository<ReadData> mRepo;
    bool mProduceFinished;
    vector<Mutation> mutationList;
    vector<Match*> *mutationMatches;
    std::mutex mMutationMtx;
    int mThreadNum;
    RollingHash* mRollingHash;
    vector<Read*> mReadToDelete;
    vector<char*> mBufToDelete;
};

#include "scanner-impl.h"

#endif
