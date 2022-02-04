#include "multihtmlreporter.h"
#include "htmlreporter.h"
#include "common.h"
#include <chrono>
#include "globalsettings.h"
#include <sys/stat.h>

MultiHtmlReporter::MultiHtmlReporter(string filename, vector<Mutation>& mutationList, vector<Match*> *mutationMatches){
    mMutationList = mutationList;
    mMutationMatches = mutationMatches;
    mFilename = filename;
    mFolderPath = mFilename + ".files";
    mFolderName = basename(mFilename) + ".files";
    mkdir(mFolderPath.c_str(), 0777);
    stat();
}

MultiHtmlReporter::~MultiHtmlReporter(){
}

void MultiHtmlReporter::stat(){
    mTotalCount = 0;
    for(size_t m=0; m<mMutationList.size(); m++) {
        vector<Match*> matches = mMutationMatches[m];
        if((ssize_t)matches.size()>=GlobalSettings::minReadSupport) {
            mTotalCount++;
            string chr = mMutationList[m].mChr;
            if(mChrCount.count(chr)==0)
                mChrCount[chr]=1;
            else
                mChrCount[chr]++;
        }
    }
}

void MultiHtmlReporter::run() {
    printCSS();
    printJS();
    printChrHtml();
    printMutationHtml();
    printMainFrame();
    printIndexPage();
    printMainPage();
}

void MultiHtmlReporter::printMainFrame() {
    ofstream file;
    file.open(mFilename.c_str(), ifstream::out);
    file << "<html><head><title>MutScan " << MUTSCAN_VER << " report " << "at " << getCurrentSystemTime() << " </title></head><frameset border=0 cols='250, *' frameborder='yes' framespacing='1'><frame name='_index' src='";
    file << mFolderName + "/index.html";
    file << "'/><frame name='_main' src='";
    file << mFolderName + "/main.html";
    file << "'/></frameset></html>";
    file.close();
}

void MultiHtmlReporter::printMainPage() {
    ofstream file;
    string mainFile = mFolderPath + "/main.html";
    file.open(mainFile.c_str(), ifstream::out);
    printHeader(file);
    printAllChromosomeLink(file);
    printFooter(file);
    file.close();
}

void MultiHtmlReporter::printAllChromosomeLink(ofstream& file) {
    bool found = false;
    map<string, int>::iterator iter;
    file << "<ul id='menu'>";
    file << "<div style='font-size:12px;padding-top:20px;text-align:left;color:#aaaaaa'>Mutations found of all chromosomes:</div>";
    for(size_t m=0; m<mMutationList.size(); m++) {
        vector<Match*> matches = mMutationMatches[m];
        if((ssize_t)matches.size()>=GlobalSettings::minReadSupport) {
            found = true;
            string chr = mMutationList[m].mChr;
            string filename = chr + "/" + to_string(m) + ".html";
            file << "<li class='menu_item'><a href='" << filename << "'>" << mMutationList[m].mName;
            file<< " (" << matches.size() << " reads support, " << Match::countUnique(matches) << " unique)" 
            << " </a></li>";
        }
    }
    if(!found) {
        file << "<span style='size:28px;font-weight:bold;color:blue;line-height:30px;'>MutScan didn't find any mutation</span>";
    }
    file << "</ul>";
}

void MultiHtmlReporter::printChrLink(ofstream& file, string chr) {
    bool found = false;
    for(size_t m=0; m<mMutationList.size(); m++) {
        vector<Match*> matches = mMutationMatches[m];
        if((ssize_t)matches.size()>=GlobalSettings::minReadSupport) {
            found = true;
            if(chr == mMutationList[m].mChr) {
                string filename = chr + "/" + to_string(m) + ".html";
                file << "<li class='menu_item'><a href='" << filename << "'>" << mMutationList[m].mName;
                file<< " (" << matches.size() << " reads support, " << Match::countUnique(matches) << " unique)" 
                << " </a></li>";
            }
        }
    }
    if(!found) {
        file << "<span style='size:28px;font-weight:bold;color:blue;line-height:30px;'>MutScan didn't find any mutation</span>";
    }
}

void MultiHtmlReporter::printMutationHtml() {
    for(size_t m=0; m<mMutationList.size(); m++) {
        vector<Match*> matches = mMutationMatches[m];
        if((ssize_t)matches.size()>=GlobalSettings::minReadSupport) {
            string chr = mMutationList[m].mChr;
            string folder = mFolderPath + "/" + chr;
            string filename = folder + "/" + to_string(m) + ".html";
            vector<Mutation> mutList;
            mutList.push_back(mMutationList[m]);
            HtmlReporter hr(filename, mutList, mMutationMatches+m, true);
            hr.run();
        }
    }
}

void MultiHtmlReporter::printIndexPage() {
    ofstream file;
    string indexFile = mFolderPath + "/index.html";
    file.open(indexFile.c_str(), ifstream::out);
    printHeader(file);
    file << "<div id='logo' style='text-align:center;'> <span style='font-size:30px;font-weight:bold;'> MutScan </span> <span style='font-size:20px;'> " << MUTSCAN_VER << " </span> </div>";
    file << "<ul id='menu'>";
    file << "<div style='font-size:12px;padding-top:20px;color:#aaaaaa'>Mutations by chromosome:</div>";
    file << "<li class='menu_item'><a class='index' href='main.html' target='_main'>All <font color='#aaaaaa' size='-2'>(" << mTotalCount << " mutations)</font></a></li>";
    map<string, int>::iterator iter;
    for(iter= mChrCount.begin(); iter!= mChrCount.end(); iter++){
        file << "<li class='menu_item'><a class='index' href='" << iter->first << ".html' target='_main'>" << iter->first << " <font color='#aaaaaa' size='-2'>(" << iter->second << " mutations)</font></a></li>";
    }
    file << "</ul>";
    printFooter(file, false, false);
    file.close();
}

void MultiHtmlReporter::printChrHtml() {
    map<string, int>::iterator iter;
    for(iter= mChrCount.begin(); iter!= mChrCount.end(); iter++){
        string chr = iter->first;
        string folder = mFolderPath + "/" + chr;
        mkdir(folder.c_str(), 0777);
        ofstream file;
        string chrFilename = mFolderPath + "/" + chr + ".html";
        file.open(chrFilename.c_str(), ifstream::out);
        printHeader(file);
        file << "<ul id='menu'>";
        file << "<div style='font-size:12px;padding-top:20px;text-align:left;color:#aaaaaa'>Mutations found of " << chr <<":</div>";
        printChrLink(file, chr);
        file << "</ul>";
        printFooter(file, false);
        file.close();
    }
}

void MultiHtmlReporter::printHelper(ofstream& file) {
    file << "<div id='helper'><p>Helpful tips:</p><ul>";
    file << "<li> Base color indicates quality: <font color='#78C6B9'>extremely high (Q40+)</font>, <font color='#33BBE2'>high (Q30+)</font>, <font color='#666666'>moderate (Q20+)</font>, <font color='#E99E5B'>low (Q15+)</font>, <font color='#FF0000'>extremely low (0~Q14)</font> </li>";
    file << "<li> Move mouse over the base, it will show the quality value</li>";
    if(!GlobalSettings::simplifiedMode)
        file << "<li> Click on any row, the original read/pair will be displayed</li>";
    file << "<li> In first column, <i>d</i> means the edit distance of match, and --> means forward, <-- means reverse </li>";
    file << "<li> For pair-end sequencing, MutScan tries to merge each pair, and the overlapped bases will be assigned higher qualities </li>";
    file << "</ul></div>";
}

void MultiHtmlReporter::printHeader(ofstream& file){
    file << "<html><head><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\" />";
    file << "<title>MutScan report</title>";
    file << "<script type='text/javascript' src='mutscan.js'></script>";
    file << "<link type='text/css' rel='stylesheet' charset='utf-8' href='mutscan.css'/>";
    file << "</head>";
    file << "<body><div id='container'>";
}

void MultiHtmlReporter::printCSS(){
    ofstream file;
    string filename = mFolderPath + "/mutscan.css";
    file.open(filename.c_str(), ifstream::out);
    file << "td {border:1px solid #dddddd;padding-left:2px;padding-right:2px;font-size:10px;}";
    file << "table {border:1px solid #999999;padding:2x;border-collapse:collapse;}";
    file << "img {padding:30px;}";
    file << "#menu {font-family:Consolas, 'Liberation Mono', Menlo, Courier, monospace;}";
    file << "#menu a {color:#0366d6; font-size:15px;line-height:22px;text-decoration:none;font-family:-apple-system, BlinkMacSystemFont, 'Segoe UI', Helvetica, Arial, sans-serif, 'Apple Color Emoji', 'Segoe UI Emoji', 'Segoe UI Symbol'}";
    file << ".index {font-weight:600;font-siz3:18px;line-height:25px;}";
    file << "a:visited {color: #999999}";
    file << ".alignleft {text-align:left;}";
    file << ".alignright {text-align:right;}";
    file << ".header {color:#ffffff;padding:1px;height:20px;background:#000000;}";
    file << ".figuretitle {color:#996657;font-size:20px;padding:50px;}";
    file << "#container {text-align:center;padding:1px;font-family:Arail,'Liberation Mono', Menlo, Courier, monospace;}";
    file << "#menu {padding-top:10px;padding-bottom:10px;text-align:left;}";
    file << ".menu_item {text-align:left;padding-top:2px;font-size:18px;}";
    file << ".highlight {text-align:left;padding-top:30px;padding-bottom:30px;font-size:20px;line-height:35px;}";
    file << ".mutation_head {text-align:left;color:#0092FF;font-family:Arial;padding-top:20px;padding-bottom:5px;}";
    file << ".mutation_block {}";
    file << ".match_brief {font-size:8px}";
    file << ".mutation_point {color:#FFCCAA}";
    file << "#helper {text-align:left;border:1px dotted #fafafa;color:#777777;font-size:12px;}";
    file << "#footer {text-align:left;padding-left:10px;padding-top:20px;color:#999999;font-size:10px;}";
    file.close();
}

void MultiHtmlReporter::printJS(){
    ofstream file;
    string filename = mFolderPath + "/mutscan.js";
    file.open(filename.c_str(), ifstream::out);
    file << "function toggle(targetid){ \n\
                if (document.getElementById){ \n\
                    target=document.getElementById(targetid); \n\
                        if (target.style.display=='table-row'){ \n\
                            target.style.display='none'; \n\
                        } else { \n\
                            target.style.display='table-row'; \n\
                        } \n\
                } \n\
            }";
    file << "function toggle_target_list(targetid){ \n\
                if (document.getElementById){ \n\
                    target=document.getElementById(targetid); \n\
                        if (target.style.display=='block'){ \n\
                            target.style.display='none'; \n\
                            document.getElementById('target_view_btn').value='view';\n\
                        } else { \n\
                            document.getElementById('target_view_btn').value='hide';\n\
                            target.style.display='block'; \n\
                        } \n\
                } \n\
            }";
    file.close();
}

string MultiHtmlReporter::getCurrentSystemTime()
{
  auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  struct tm* ptm = localtime(&tt);
  char date[60] = {0};
  sprintf(date, "%d-%02d-%02d      %02d:%02d:%02d",
    (int)ptm->tm_year + 1900,(int)ptm->tm_mon + 1,(int)ptm->tm_mday,
    (int)ptm->tm_hour,(int)ptm->tm_min,(int)ptm->tm_sec);
  return std::string(date);
}

extern string command;

void MultiHtmlReporter::printFooter(ofstream& file, bool printTargetList, bool printCommand){
    file << "\n<div id='footer'> ";
    if(printCommand)
        file << "<p>"<<command<<"</p>";
    if(printTargetList)
        printScanTargets(file);
    file << getCurrentSystemTime() << " </div>";
    file << "</div></body></html>";
}

void MultiHtmlReporter::printScanTargets(ofstream& file){
    file << "\n<div id='targets'> ";
    file << "<p> scanned " << mMutationList.size() << " mutation spots...<input type='button' id='target_view_btn', onclick=toggle_target_list('target_list'); value='show'></input></p>";
    file << "<ul id='target_list' style='display:none'>";
    int id=0;
    for(size_t i=0;i<mMutationList.size();i++){
        id++;
        file<<"<li> " << id << ", " << mMutationList[i].mName << "</li>";
    }
    file << "</ul></div>";
}