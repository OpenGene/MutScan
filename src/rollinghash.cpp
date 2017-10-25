#include "rollinghash.h"
#include "builtinmutation.h"
#include "util.h"
#include <memory.h>

// we use 512M memory
const int BLOOM_FILTER_LENGTH = (1<<29);

RollingHash::RollingHash(int window, bool allowTwoSub) {
    mWindow = min(48, window);
    mAllowEditDistanceIs2 = allowTwoSub;
    mBloomFilterArray = new char[BLOOM_FILTER_LENGTH];
    memset(mBloomFilterArray, 0, BLOOM_FILTER_LENGTH * sizeof(char));
}

RollingHash::~RollingHash() {
    delete mBloomFilterArray;
    mBloomFilterArray = NULL;
}

void RollingHash::initMutations(vector<Mutation>& mutationList) {
    // for memory and speed consideration
    // when dealing with big mutation list (usually from VCF), we apply more strict matching
    if(mutationList.size() > 5000)
        mAllowEditDistanceIs2 = false;

    for(int i=0; i<mutationList.size(); i++) {
        Mutation m = mutationList[i];
        string s1 = m.mLeft + m.mCenter + m.mRight;
        add(s1, i, !m.isSmallIndel());
        const int margin = 4;
        // handle the case mRight is incomplete, but at least margin
        int left = max((size_t)mWindow+2, m.mLeft.length() + m.mCenter.length() + margin+1);
        string s2 = s1.substr(left - (mWindow+2), mWindow+2);
        add(s2,i, !m.isSmallIndel());
        //handle the case mleft is incomplete, but at least margin
        int right = min(s1.length() - (mWindow+2), m.mLeft.length()-margin-1);
        string s3 = s1.substr(right, mWindow+2);
        add(s3,i, !m.isSmallIndel());
    }
}

map<long, vector<int> > RollingHash::getKeyTargets() {
    return mKeyTargets;
}

bool RollingHash::add(string s, int target, bool allowIndel) {
    if(s.length() < mWindow + 2)
        return false;

    int center = s.length() / 2;
    int start = center - mWindow / 2;
    
    // mutations cannot happen in skipStart to skipEnd
    int skipStart = center - 1;
    int skipEnd = center + 1;

    const char* data = s.c_str();

    long* hashes = new long[mWindow];
    memset(hashes, 0, sizeof(long)*mWindow);

    long* accum = new long[mWindow];
    memset(accum, 0, sizeof(long)*mWindow);

    // initialize
    long origin = 0;
    for(int i=0; i<mWindow; i++) {
        hashes[i] = hash(data[start + i], i);
        origin += hashes[i];
        accum[i] = origin;
    }
    addHash(origin, target);

    const char bases[4] = {'A', 'T', 'C', 'G'};

    // make subsitution hashes, we allow up to 2 sub mutations
    for(int i=0; i<mWindow; i++) {
        if(i+start >= skipStart && i+start <= skipEnd )
            continue;
        for(int b1=0; b1<4; b1++){
            char base1 = bases[b1];
            if(base1 == data[start + i])
                continue;

            long mut1 = origin - hash(data[start + i], i) + hash(base1, i);
            addHash(mut1, target);
            /*cout<<mut1<<":"<<i<<base1<<":";
            for(int p=0; p<mWindow; p++) {
                if(p==i)
                    cout<<base1;
                else
                    cout<<data[start + p];
            }
            cout<<endl;*/

            if(mAllowEditDistanceIs2) {
                for(int j=i+1; j<mWindow; j++) {
                    if(j+start >= skipStart && j+start <= skipEnd )
                        continue;
                    for(int b2=0; b2<4; b2++){
                        char base2 = bases[b2];
                        if(base2 == data[start + j])
                            continue;
                        long mut2 = mut1 - hash(data[start + j], j) + hash(base2, j);
                        addHash(mut2, target);
                        /*cout<<mut2<<":"<<i<<base1<<j<<base2<<":";
                        for(int p=0; p<mWindow; p++) {
                            if(p==i)
                                cout<<base1;
                            else if(p==j)
                                cout<<base2;
                            else
                                cout<<data[start + p];
                        }
                        cout<<endl;*/
                    }
                }
            }
        }
    }

    int altForDel = start - 1;
    long altVal = hash(data[altForDel], 0);

    if(allowIndel && mAllowEditDistanceIs2) {
        // make indel hashes, we only allow 1 indel
        for(int i=0; i<mWindow; i++) {
            if(i+start >= skipStart && i+start <= skipEnd )
                continue;
            // make del of i first
            long mutOfDel;
            if (i==0)
                mutOfDel = origin - accum[i] + altVal;
            else
                mutOfDel = origin - accum[i] + (accum[i-1]<<1) + altVal;
            if(mutOfDel != origin)
                addHash(mutOfDel, target);

            // make insertion
            for(int b=0; b<4; b++){
                char base = bases[b];
                // shift the first base
                long mutOfIns = origin - accum[i] + hash(base, i) + ((accum[i] - hashes[0]) >> 1);
                if(mutOfIns != origin && mutOfIns != mutOfDel){
                    addHash(mutOfIns, target);
                    /*cout << mutOfIns<<", insert at " << i << " with " << base << ": ";
                    for(int p=1;p<=i;p++)
                        cout << data[start + p];
                    cout << base;
                    for(int p=i+1;p<mWindow;p++)
                        cout << data[start + p];
                    cout << endl;*/
                }
            }
        }
    }

    delete hashes;
    delete accum;

    return true;
}

map<int, int> RollingHash::hitTargets(const string s) {
    map<int, int> ret;
    if(s.length() < mWindow)
        return ret;

    const char* data = s.c_str();

    // initialize
    long curHash = 0;
    for(int i=0; i<mWindow; i++) {
        long h = hash(data[i], i);
        curHash += h;
    }
    addHit(ret, curHash);

    for(int i=mWindow; i<s.length(); i++) {
        curHash = ((curHash - hash(data[i - mWindow], 0))>>1) + hash(data[i], mWindow-1);
        addHit(ret, curHash);
    }

    return ret;
}

inline void RollingHash::addHit(map<int, int>& ret, long hash) {
    //update bloom filter array
    const long bloomFilterFactors[3] = {1713137323, 371371377, 7341234131};
    for(int b=0; b<3; b++) {
        if(mBloomFilterArray[(bloomFilterFactors[b] * hash) & (BLOOM_FILTER_LENGTH-1)] == 0 )
            return;
    }

    if(mKeyTargets.count(hash)) {
        for(int i=0; i<mKeyTargets[hash].size(); i++) {
            int val = mKeyTargets[hash][i];
            if(ret.count(val)) {
                //cout << "-";
                ret[val]++;
            }
            else {
                //cout << ".";
                ret[val]=1;
            }
        }
    }
}

void RollingHash::addHash(long hash, int target) {
    if(mKeyTargets.count(hash) == 0)
        mKeyTargets[hash] = vector<int>();
    else {
        for(int i=0; i<mKeyTargets[hash].size(); i++) {
            if(mKeyTargets[hash][i] == target)
                return ;
        }
    }

    mKeyTargets[hash].push_back(target);

    //update bloom filter array
    const long bloomFilterFactors[3] = {1713137323, 371371377, 7341234131};
    for(int b=0; b<3; b++) {
        mBloomFilterArray[(bloomFilterFactors[b] * hash) & (BLOOM_FILTER_LENGTH-1) ] = 1;
    }
}

inline long RollingHash::char2val(char c) {
    switch (c) {
        case 'A':
            return 517;
        case 'T':
            return 433;
        case 'C':
            return 1123;
        case 'G':
            return 127;
        case 'N':
            return 1;
        default:
            return 0;
    }
}

inline long RollingHash::hash(char c, int pos) {
    long val = char2val(c);
    const long num = 2;
    return val * (num << pos );
}

void RollingHash::dump() {
    map<long, vector<int> >::iterator iter;
    for(iter= mKeyTargets.begin(); iter!=mKeyTargets.end(); iter++) {
        if(iter->second.size() < 2)
            continue;
        cout << iter->first << endl;
        for(int i=0; i<iter->second.size(); i++)
            cout << iter->second[i] << "\t";
        cout << endl;
    }
}
bool RollingHash::test(){
    vector<Mutation> mutationList = Mutation::parseBuiltIn();
    RollingHash rh(48);
    rh.initMutations(mutationList);
    bool result = true;
    for(int i=0; i<mutationList.size(); i++) {
        Mutation m = mutationList[i];
        string s = m.mLeft + m.mCenter + m.mRight;
        map<int, int> targets = rh.hitTargets(s);
        cout << i << ", " << s << endl;
        bool found = false;
        map<int, int>::iterator iter;
        for(iter=targets.begin(); iter!=targets.end(); iter++) {
            int t = iter->first;
            int count = iter->second;
            cout << t << "\t";
            if(t == i)
                found = true;
        }
        cout << endl;
        result &= found;
    }
    return result;

}
