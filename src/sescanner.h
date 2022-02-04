#ifndef SE_SCANNNER_H
#define SE_SCANNNER_H

#include "scanner.h"

template <>
class reader_trait<Read> {
public:
    typedef FastqReader FastqReaderType;
    static constexpr const char* name="reads";
};

class SingleEndScanner : public Scanner<Read, SingleEndScanner> {
public:
    SingleEndScanner(string mutationFile, string refFile, string read1File, string html="", string json = "", int threadnum=1);
    ~SingleEndScanner();

private:
    friend Scanner<Read, SingleEndScanner>;
    bool scanNextEnd(ReadPack<Read>* pack);
    reader_trait<Read>::FastqReaderType *fastqReader() {
        return new FastqReader(mRead1File);
    }
};

#endif
