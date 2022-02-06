#ifndef PE_SCANNNER_H
#define PE_SCANNNER_H

#include "scanner.h"

template <>
struct reader_trait<ReadPair> {
    typedef FastqReaderPair FastqReaderType;
    static constexpr const char* name="pairs";
};

class PairEndScanner : public Scanner<ReadPair, PairEndScanner> {
public:
    PairEndScanner(string mutationFile, string refFile, string read1File, string read2File, string html="", string json = "", int threadnum=1);
    ~PairEndScanner();

private:
    friend Scanner<ReadPair, PairEndScanner>;
    bool scanNextEnd(ReadPack<ReadPair>* pack);
    reader_trait<ReadPair>::FastqReaderType *fastqReader() {
        return new FastqReaderPair(mRead1File, mRead2File);
    }
};


#endif
