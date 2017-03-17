#include <stdio.h>
#include "fastqreader.h"
#include "unittest.h"
#include "mutscan.h"
#include <time.h>
#include "cmdline.h"
#include <sstream>
#include "util.h"
#include "globalsettings.h"

string command;

int main(int argc, char* argv[]){
    if (argc == 2 && strcmp(argv[1], "test")==0){
        UnitTest tester;
        tester.run();
        return 0;
    }
    cmdline::parser cmd;
    cmd.add<string>("read1", '1', "read1 file name", true, "");
    cmd.add<string>("read2", '2', "read2 file name", false, "");
    cmd.add<string>("mutation", 'm', "mutation file name, can be a CSV format or a VCF format", false, "");
    cmd.add<string>("ref", 'r', "reference fasta file name (only needed when mutation file is a VCF)", false, "");
    cmd.add<string>("html", 'h', "filename of html report, no html report if not specified", false, "");
    cmd.add<int>("thread", 't', "worker thread number, default is 4", false, 4);
    cmd.add("mark", 'k', "when mutation file is a vcf file, --mark means only process the records with FILTER column is M");
    cmd.add("legacy", 'l', "use legacy mode, usually much slower but may be able to find a little more reads in certain case");
    cmd.parse_check(argc, argv);
    string r1file = cmd.get<string>("read1");
    string r2file = cmd.get<string>("read2");
    string mutationFile = cmd.get<string>("mutation");
    string html = cmd.get<string>("html");
    string refFile = cmd.get<string>("ref");
    int threadNum = cmd.get<int>("thread");

    if(ends_with(refFile, ".gz") || ends_with(refFile, ".gz")) {
        cout << "reference fasta file should not be compressed.\nplease unzip "<<refFile<<" and try again."<<endl;
        exit(-1);
    }

    bool markedOnlyForVCF = cmd.exist("mark");
    GlobalSettings::setMarkedOnlyForVCF(markedOnlyForVCF);

    bool legacyMode = cmd.exist("legacy");
    GlobalSettings::setLegacyMode(legacyMode);

    stringstream ss;
    for(int i=0;i<argc;i++){
        ss << argv[i] << " ";
    }
    command = ss.str();

    check_file_valid(r1file);
    if(r2file != "")
        check_file_valid(r2file);
    if(mutationFile != "")
        check_file_valid(mutationFile);
    if(refFile != "")
        check_file_valid(refFile);
    // if the mutation file is a vcf, then the reference should be provided
    if(ends_with(mutationFile, ".vcf") || ends_with(mutationFile, ".VCF") || ends_with(mutationFile, ".Vcf")){
        if(refFile == "") {
            cerr << "You should specify the reference fasta file by -r <ref.fa>, because your mutation file (-m) is a VCF"<<endl;
            exit(-1);
        }
    }

    clock_t t1 = clock();
    MutScan scanner(mutationFile, refFile, r1file, r2file, html, threadNum);
    scanner.scan();
    clock_t t2 = clock();
    printf("\n%s\n", command.c_str());
    printf("Mutscan v%s, time used: %f ms\n", MUTSCAN_VER, (t2-t1)/1000.0);
}