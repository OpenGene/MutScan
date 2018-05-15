#include "jsonreporter.h"
#include "common.h"
#include <chrono>
#include "globalsettings.h"

JsonReporter::JsonReporter(string filename, vector<Mutation>& mutationList, vector<Match*> *mutationMatches){
    mMutationList = mutationList;
    mMutationMatches = mutationMatches;
    mFilename = filename;
    mFile.open(mFilename.c_str(), ifstream::out);
}

JsonReporter::~JsonReporter(){
    mFile.close();
}

extern string getCurrentSystemTime();
extern string command;

void JsonReporter::run() {
    mFile << "{" << endl;
    mFile << "\t\"command\":\"" << command << "\"," << endl;
    mFile << "\t\"version\":\"" << MUTSCAN_VER << "\"," << endl;
    mFile << "\t\"time\":\"" << getCurrentSystemTime() << "\"," << endl;
    mFile << "\t\"mutations\":{";

    bool isFirstMut = true;
    for(int i=0;i<mMutationList.size();i++){
        Mutation mut = mMutationList[i];
        vector<Match*> matches = mMutationMatches[i];
        if(matches.size()>=GlobalSettings::minReadSupport){
            if(isFirstMut) {
                mFile << endl;
                isFirstMut = false;
            }
            else
                mFile << "," << endl;

            mFile << "\t\t\"" <<  mut.mName << "\":{" << endl;
            mFile << "\t\t\t\"" <<  "chr" << "\":" << "\"" << mut.mChr << "\"," << endl;
            mFile << "\t\t\t\"" <<  "ref" << "\":[" << "\"" << mut.mLeft << "\"," << "\"" << mut.mCenter << "\"," << "\"" << mut.mRight << "\"]," << endl;
            mFile << "\t\t\t\"" <<  "reads" << "\":[" << endl;
            for(int m=0; m<matches.size(); m++){
                mFile << "\t\t\t\t{" << endl;
                mFile << "\t\t\t\t\t\"breaks\":"; 
                matches[m]->printBreaksToJson(mFile, mut.mLeft.length(), mut.mCenter.length(), mut.mRight.length());
                mFile << ", " << endl;
                matches[m]->printReadToJson(mFile, "\t\t\t\t\t");
                mFile << "\t\t\t\t}";
                if(m!=matches.size()-1)
                    mFile << ",";
                mFile << endl;
            }
            mFile << "\t\t\t]" << endl;
            mFile << "\t\t}";
        }

    }
    mFile << endl;
    mFile << "\t}" << endl;
    mFile << "}" << endl;
}