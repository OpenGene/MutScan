#include "mutscan.h"
#include "fastqreader.h"
#include <iostream>
#include "htmlreporter.h"
#include "sescanner.h"
#include "pescanner.h"
#include "util.h"
#include "globalsettings.h"

MutScan::MutScan(string mutationFile, string refFile, string read1File, string read2File, string html, string json, int threadNum){
    mRead1File = read1File;
    mRead2File = read2File;
    mMutationFile = mutationFile;
    mRefFile = refFile;
    mHtmlFile = html;
    mJsonFile = json;
    mThreadNum = threadNum;
}

bool MutScan::scan(){
    if(mRead2File != ""){
        //return scanPairEnd();
        PairEndScanner pescanner( mMutationFile, mRefFile, mRead1File, mRead2File, mHtmlFile, mJsonFile, mThreadNum);
        return pescanner.scan();
    }
    else{
        //return scanSingleEnd();
        SingleEndScanner sescanner( mMutationFile, mRefFile, mRead1File, mHtmlFile, mJsonFile, mThreadNum);
        return sescanner.scan();
    }
}

void MutScan::evaluateSimplifiedMode(string r1file, string r2file, int mutationNum) {
    if(mutationNum < 10000) {
        GlobalSettings::setSimplifiedMode(false);
        return ;
    }
    // use another ifstream to not affect current reader
    ifstream is(r1file);
    is.seekg (0, is.end);
    long bytes = is.tellg();

    if(r2file != "")
        bytes *= 2;

    // here we consider gz file for FASTQ has a compression rate of 3
    if(ends_with(r1file, ".gz"))
        bytes *= 3;

    // enable simplified mode for over 50G FASTQ + 10,000 mutations
    if(bytes > 50L * 1024L * 1024L * 1024L) {
        if(GlobalSettings::verbose)
            cerr << "Simplified mode is enabled automatically..."<<endl;
        GlobalSettings::setSimplifiedMode(true);
    }
    else {
        GlobalSettings::setSimplifiedMode(false);
    }

}