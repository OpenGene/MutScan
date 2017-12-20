#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

class GlobalSettings{
public:
    GlobalSettings();

public:
    inline static void setMarkedOnlyForVCF(bool flag){
        markedOnlyForVCF = flag;
    }
    inline static void setLegacyMode(bool flag){
        legacyMode = flag;
    }
    inline static void setStandaloneHtml(bool flag){
        standaloneHtml = flag;
    }
    inline static void setMinReadSupport(int val){
        minReadSupport = val;
    }
    inline static void setProcessingVCF(bool flag){
        processingVCF = flag;
    }
    inline static void setVerbose(bool flag){
        verbose = flag;
    }
    inline static void setSimplifiedMode(bool flag){
        simplifiedMode = flag;
    }
    inline static void setSimplifiedModeToEvaluate(bool flag){
        simplifiedModeToEvaluate = flag;
    }

public:
    static bool markedOnlyForVCF;
    static bool legacyMode;
    static bool standaloneHtml;
    static int minReadSupport;
    static bool processingVCF;
    static bool verbose;
    static bool simplifiedMode;
    static bool simplifiedModeToEvaluate;
};


#endif