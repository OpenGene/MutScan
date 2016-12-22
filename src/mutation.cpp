#include "mutation.h"
#include "editdistance.h"
#include <iostream>
#include <fstream>
#include "util.h"
#include <string.h>
#include "builtinmutation.h"
#include <sstream>

Mutation::Mutation(string name, string left, string center, string right){
	//we shift some bases from left and right to center to require 100% match of these bases
    mShift = 2;
    mLeft = left.substr(0, left.length()-mShift);
	mCenter = left.substr(left.length()-mShift, mShift) + center + right.substr(0, mShift);
    mRight = right.substr(mShift, right.length()-mShift);
    mPattern = left + center + right;
    mName = name;
}

Match* Mutation::searchInRead(Read* r, int distanceReq, int qualReq){
    char phredQualReq= (char)(qualReq + 33);
    int readLen = r->mSeq.length();
    int lLen = mLeft.length();
    int cLen = mCenter.length();
    int rLen = mRight.length();
    int pLen = mPattern.length();
    string seq = r->mSeq.mStr;
    const char* seqData = seq.c_str();
    const char* centerData = mCenter.c_str();
    const char* patternData = mPattern.c_str();
    const char* qualData = r->mQuality.c_str();
    for(int start = lLen; start + cLen + rLen < readLen; start++){
        // check string identity in a fast way
        bool identical = true;
        for (int i=0;i<cLen;i++){
            if (seqData[start + i] != centerData[i]){
                identical = false;
                break;
            }
        }
        if(!identical)
            continue;

        // check quality in a fast way
        bool qualityPassed = true;
        for (int i=mShift; i<cLen-mShift; i++){
            if (qualData[start + i] < phredQualReq){
                qualityPassed = false;
                break;
            }
        }
        if(!qualityPassed)
            continue;
        int ed = edit_distance(seqData + start - lLen, pLen, patternData, pLen);
        if ( ed <= distanceReq){
            return new Match(r, start-lLen, ed);
        }
    }
    return NULL;
}

vector<Mutation> Mutation::parseCsv(string filename) {
    ifstream file;
    file.open(filename.c_str(), ifstream::in);
    const int maxLine = 1000;
    char line[maxLine];
    vector<Mutation> mutations;
    while(file.getline(line, maxLine)){
        // trim \n, \r or \r\n in the tail
        int readed = strlen(line);
        if(readed >=2 ){
            if(line[readed-1] == '\n' || line[readed-1] == '\r'){
                line[readed-1] = '\0';
                if(line[readed-2] == '\r')
                    line[readed-2] = '\0';
            }
        }
        string linestr(line);
        vector<string> splitted;
        split(linestr, splitted, ",");
        // a valid line need 4 columns: name, left, center, right
        if(splitted.size()<4)
            continue;
        // comment line
        if(starts_with(splitted[0], "#"))
            continue;
        string name = trim(splitted[0]);
        string left = trim(splitted[1]);
        string center = trim(splitted[2]);
        string right = trim(splitted[3]);
        Mutation mut(name, left, center, right);
        mutations.push_back(mut);
    }
    file.close();
    return mutations;
}

vector<Mutation> Mutation::parseBuiltIn() {
    vector<Mutation> mutations;
    vector<string> lines;
    split(BUILT_IN_MUTATIONS, lines, "\n");
    for(int i=0;i<lines.size();i++){
        string linestr = lines[i];
        vector<string> splitted;
        split(linestr, splitted, ",");
        // a valid line need 4 columns: name, left, center, right
        if(splitted.size()<4)
            continue;
        // comment line
        if(starts_with(splitted[0], "#"))
            continue;
        string name = trim(splitted[0]);
        string left = trim(splitted[1]);
        string center = trim(splitted[2]);
        string right = trim(splitted[3]);
        Mutation mut(name, left, center, right);
        mutations.push_back(mut);
    }
    return mutations;
}

vector<Mutation> Mutation::parseVcf(string vcfFile, string refFile, bool maskedOnly) {
    vector<Mutation> mutations;
    VcfReader vr(vcfFile);
    vr.readAll();
    vector<Variant> variants = vr.variants();

    FastaReader fr(refFile);
    fr.readAll();
    map<string, string> ref = fr.contigs();

    for(int i=0;i<variants.size();i++) {
        Variant& v = variants[i];
        // skip the unmasked if maskedOnly flag is set true
        if(maskedOnly && (v.filter!="m" || v.filter!="M"))
            continue;
        string chrom = "chr" + v.chrom;
        // the contig is not in reference
        if(ref.count(chrom) == 0)
            continue;
        // the variant is out of this contig, or in the front or tail
        // note that VCF is 1-based, and string is 0-based
        if(v.pos > ref[chrom].length() + 25 + 1 || v.pos < 25 + 1)
            continue;

        string gene = v.gene();
        string aa = v.aaChange();
        string cds = v.cdsChange();

        stringstream ss;
        if(gene!="")
            ss<<gene<<"_";
        if(aa!="")
            ss<<aa<<"_";
        if(cds!="")
            ss<<cds<<"_";
        ss<<chrom<<"_"<<v.pos<<"_"<<v.ref<<">"<<v.alt;
        string name = ss.str();
        string left = ref[chrom].substr(v.pos-25-1, 25);
        string center = v.alt;
        string right = ref[chrom].substr(v.pos+v.ref.length()-1, 25);
        Mutation mut(name, left, center, right);
        cout << name << ", " << left << ", " << center << ", " << right << endl;
        mutations.push_back(mut);
    }
    return mutations;
}

void Mutation::print(){
    cout<<mName<<" "<<mLeft<<" "<<mCenter<<" "<<mRight<<endl;
}

void Mutation::printHtml(ofstream& file){
    file<<mName<<" "<<mLeft<<" "<<mCenter<<" "<<mRight;
}
