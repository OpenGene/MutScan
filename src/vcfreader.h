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
#include "util.h"

using namespace std;

class Variant{
public:
    inline string gene() {
        int start = find_with_right_pos(info, "Gene.refGene=");
        if (start<0){
            start = find_with_right_pos(info, "GENE=");
            if(start<0)
                return "";
        }
        int end1 = info.find(";", start);
        int end2 = info.find(":", start);
        int end = 0;
        if(end1>=0)
            end = end1;
        if(end2>=0 && (end1<0 || end2<end1))
            end=end2;
        if(start>=0 && end>start){
            return info.substr(start, end-start);
        }
        return "";
    }
    inline string cdsChange(){
        int start = find_with_right_pos(info, "CDS=c.");
        if (start<0){
            start = find_with_right_pos(info, ":c.");
            if(start<0)
                return "";
        }
        int end1 = info.find(";", start);
        int end2 = info.find(":", start);
        int end = 0;
        if(end1>=0)
            end = end1;
        if(end2>=0 && (end1<0 || end2<end1))
            end=end2;
        if(start>=0 && end>start){
            return "c."+info.substr(start, end-start);
        }
        return "";
    }
    inline string aaChange(){
        int start = find_with_right_pos(info, "AA=p.");
        if (start<0){
            start = find_with_right_pos(info, ":p.");
            if(start<0)
                return "";
        }
        int end1 = info.find(";", start);
        int end2 = info.find(":", start);
        int end = 0;
        if(end1>=0)
            end = end1;
        if(end2>=0 && (end1<0 || end2<end1))
            end=end2;
        if(start>=0 && end>start){
            return "p."+info.substr(start, end-start);
        }
        return "";
    }
public:
    string chrom;
    int pos;
    string id;
    string ref;
    string alt;
    string qual;
    string filter;
    string info;
    string format;
};

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

