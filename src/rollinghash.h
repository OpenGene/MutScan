#ifndef ROLLINGHASH_H
#define ROLLINGHASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include <iostream>

using namespace std;

class RollingHash{
public:
    RollingHash(int window = 50, bool allowTwoSub = true);

public:
    bool add(string s, void* target);
    void addHash(long hash, void* target);
    inline static long char2val(char c);
    inline static long hash(char c, int pos);
    void dump();
    static bool test();

private:
    map<long, vector<void*> > mKeyTargets;
    int mWindow;
    bool mAllowTwoSub;

};


#endif