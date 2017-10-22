#include "mutation.h"
#include "editdistance.h"
#include <iostream>
#include <fstream>
#include "util.h"
#include <string.h>
#include "builtinmutation.h"
#include <sstream>
#include "globalsettings.h"

Mutation::Mutation(string name, string left, string center, string right, string chr){
	//we shift some bases from left and right to center to require 100% match of these bases
    mShift = 0;
    if(center.length() == 0)
        mShift = 2;
    mLeft = left.substr(0, left.length()-mShift);
	mCenter = left.substr(left.length()-mShift, mShift) + center + right.substr(0, mShift);
    mRight = right.substr(mShift, right.length()-mShift);
    mPattern = left + center + right;
    mName = name;
    mSmallIndel = false;
    mChr = chr;
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
    // we should ignore the mutations in the exact edge since there usualy exists errors
    const int margin = 2;
    for(int start = margin; start + cLen + margin <= readLen; start++){
        int lComp = min(start, lLen);
        int rComp = min(readLen - (start+cLen), rLen);
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
        int edLen = min(pLen - (lLen - lComp), readLen - (start - lComp));
        // too short
        if(edLen < 15)
            continue;
        // if on left edge, and it is too short, we require exact match
        if(lComp < 10) {
            if(seq.substr(0, lComp) != mLeft.substr(lLen - lComp, lComp))
                continue ;
        }
        // if on right edge, and it is too short, we require exact match
        if(rComp < 10) {
            if(seq.substr(readLen-rComp, rComp) != mRight.substr(0, rComp))
                continue ;
        }
        int dis = 0;
        if(mSmallIndel)
            dis = hamming_distance(seqData + start - lComp, edLen, patternData + lLen - lComp, edLen);
        else
            dis = edit_distance(seqData + start - lComp, edLen, patternData + lLen - lComp, edLen);

        // for small indel, we should apply more strict strategy
        // we allow up to 1 mismatch
        if(mSmallIndel){
            distanceReq = min(1, distanceReq);
        }

        if ( dis <= distanceReq){
            // check if this is due to bad alignment
            if(dis >= 2 && mShift==0) {
                // if this is caused by an indel, then indel should not happen around center
                // we check half of the pattern
                int noIndelLeft = min(lLen / 2, lComp);
                if(rComp < rLen/2)
                    noIndelLeft = lLen/2 - rComp;
                int noIndelDis = hamming_distance(seqData + start - noIndelLeft, edLen/2, patternData + lLen - noIndelLeft, edLen/2);
                
                if(noIndelDis > 2)
                    continue;
            }
            return new Match(r, start, dis);
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
        string chr = "unspecified";
        if(splitted.size()>4)
            chr = trim(splitted[4]);
        Mutation mut(name, left, center, right, chr);
        if(left.length()<15){
            cerr << "WARNING: skip following mutation since its left part < 15bp"<<endl<<"\t";
            mut.print();
        }
        else if(right.length()<15){
            cerr << "WARNING: skip following mutation since its right part < 15bp"<<endl<<"\t";
            mut.print();
        }
        else if(left.length() + center.length() + right.length() < 40){
            cerr << "WARNING: skip following mutation since its (left+center+right) < 40bp"<<endl<<"\t";
            mut.print();
        }
        else {
            mutations.push_back(mut);
        }
    }
    file.close();
    if(mutations.size() <= 0){
        cerr<<"No mutation will be scanned"<<endl;
    }
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
        string chr = "unspecified";
        if(splitted.size()>4)
            chr = trim(splitted[4]);
        Mutation mut(name, left, center, right, chr);
        mutations.push_back(mut);
    }
    if(mutations.size() <= 0){
        cerr<<"No mutation will be scanned"<<endl;
    }
    return mutations;
}

vector<Mutation> Mutation::parseVcf(string vcfFile, string refFile) {
    vector<Mutation> mutations;
    VcfReader vr(vcfFile);
    vr.readAll();
    vector<Variant> variants = vr.variants();

    bool markedOnly = GlobalSettings::markedOnlyForVCF;

    /*const int vcfMax = 100;
    if(variants.size() > vcfMax && markedOnly==false){
        cerr<<"Your VCF has more than "<<vcfMax<<" records, this will make MutScan take too long to complete the scan." << endl;
        cerr<<"Please use a smaller VCF"<<endl;
        cerr<<"Or use --mark option, and mark the wanted VCF records with FILTER column as M"<<endl;
        cerr<<"Example (note the M in the FILTER column):"<<endl;
        cerr<<"#CHROM   POS     ID          REF ALT QUAL  FILTER  INFO"<<endl;
        cerr<<"1        69224   COSM3677745 A   C   .     M       This record will be scanned"<<endl;
        cerr<<"1        880950  COSM3493111 G   A   .     .       This record will be skipped"<<endl;
        cerr<<endl;
        exit(-1);
    }*/

    FastaReader fr(refFile);
    fr.readAll();
    map<string, string> ref = fr.contigs();

    for(int i=0;i<variants.size();i++) {
        Variant& v = variants[i];
        // skip the unmasked if markedOnly flag is set true
        if(markedOnly && (v.filter!="m" && v.filter!="M"))
            continue;
        string chrom = v.chrom;

        // the contig is not in reference
        if(ref.count(chrom) == 0){
            // add or remove chr to match the reference
            if(!starts_with(chrom, "chr"))
                chrom = "chr" + chrom;
            else
                chrom = chrom.substr(3, chrom.length()-3);

            if(ref.count(chrom) == 0)
                continue;
        }
        // the variant is out of this contig, or in the front or tail
        // note that VCF is 1-based, and string is 0-based
        if(v.pos > ref[chrom].length() + 25 + v.ref.length() || v.pos < 25 + v.ref.length())
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
        str2upper(left);
        string center = v.alt;
        string right = ref[chrom].substr(v.pos+v.ref.length()-1, 25);
        str2upper(right);
        Mutation mut(name, left, center, right, chrom);
        // this is a small indel
        // we don't allow indel match when searching
        int lengthDiff = abs((int)v.ref.length() - (int)v.alt.length());
        if(lengthDiff>=1 && lengthDiff<=2 )
            mut.setSmallIndel(true);
        mutations.push_back(mut);
    }
    if(mutations.size() <= 0){
        cerr<<"No mutation to be scanned"<<endl;
        if(markedOnly){
            cerr<<"You are using --mark option, you should mark the wanted VCF records with FILTER as M"<<endl;
            cerr<<"Example (note the M in the FILTER column):"<<endl;
            cerr<<"#CHROM   POS     ID          REF ALT QUAL  FILTER  INFO"<<endl;
            cerr<<"1        69224   COSM3677745 A   C   .     M       This record will be scanned"<<endl;
            cerr<<"1        880950  COSM3493111 G   A   .     .       This record will be skipped"<<endl;
        } else {
            cerr<<"Your VCF contains no valid records"<<endl;
        }
        cerr<<endl;
        exit(-1);
    }
    return mutations;
}

void Mutation::print(){
    cout<<mName<<" "<<mLeft<<" "<<mCenter<<" "<<mRight<< " "<<mChr <<endl;
}

void Mutation::printHtml(ofstream& file){
    file<<mName<<" "<<mLeft<<" "<<mCenter<<" "<<mRight<< " "<<mChr;
}

string Mutation::getCenterHtml(){
    string s = "";
    s += mCenter.substr(0, mShift);
    s += "<a class='mutation_point' title='mutation point'>" + mCenter.substr(mShift, mCenter.length()-2*mShift) + "</a>";
    s += mCenter.substr(mCenter.length()-mShift, mShift);
    return s;
}

void Mutation::setSmallIndel(bool flag) {
    mSmallIndel = flag;
}

bool Mutation::isSmallIndel() {
    return mSmallIndel;
}
