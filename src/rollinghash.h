#ifndef ROLLINGHASH_H
#define ROLLINGHASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include "mutation.h"

using namespace std;

class RollingHash{
public:
    RollingHash(int window = 40, bool allowTwoSub = true);
    ~RollingHash();
    void initMutations(vector<Mutation>& mutationList);

public:
    bool add(string s, int target);
    void addHash(long hash, int target);
    vector<int> hitTargets(const string s);
    inline void addHit(vector<int>& ret, long hash);
    map<long, vector<int> > getKeyTargets();

    inline static long char2val(char c);
    inline static long hash(char c, int pos);
    void dump();
    static bool test();

private:
    map<long, vector<int> > mKeyTargets;
    int mWindow;
    bool mAllowTwoSub;
    char* mBloomFilterArray;

};


#endif