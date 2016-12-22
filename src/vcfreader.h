#ifndef VCF_READER_H
#define VCF_READER_H

// includes
#include <cctype>
#include <clocale>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

typedef struct Variant{
    string chrom;
    int pos;
    string id;
    string ref;
    string alt;
    string qual;
    string filter;
    string info;
    string format;
} Variant;

class VcfReader
{
public:
    VcfReader(string vcfFile);
    ~VcfReader();
    bool readNext();
    void readAll();
    void printAll();

    inline vector<Variant>& variants() {
        return mVariants;
    }

    static bool test();

public:
    vector<Variant> mVariants;


private:
    string mVcfFile;
    ifstream mVcfFileStream;
};


#endif

