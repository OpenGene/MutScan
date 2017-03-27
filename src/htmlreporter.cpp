#include "htmlreporter.h"
#include "common.h"
#include <chrono>
#include "globalsettings.h"

HtmlReporter::HtmlReporter(string filename, vector<Mutation>& mutationList, vector<Match*> *mutationMatches, bool inFrame){
    mMutationList = mutationList;
    mMutationMatches = mutationMatches;
    mFilename = filename;
    mFile.open(mFilename.c_str(), ifstream::out);
    mInFrame = inFrame;
}

HtmlReporter::~HtmlReporter(){
    mFile.close();
}

void HtmlReporter::run() {
    printHeader();
    printHelper();
    printMutations();
    printMutationsJS();
    printFooter();
}

void HtmlReporter::printHelper() {
    mFile << "<div id='logo' style='text-align:center;'> <span style='font-size:30px;font-weight:bold;'> MutScan </span> <span style='font-size:20px;'> " << MUTSCAN_VER << " </span> </div>";
    mFile << "<div id='helper'><p>Helpful tips:</p><ul>";
    mFile << "<li> Base color indicates quality: <font color='#78C6B9'>extremely high (Q40+)</font>, <font color='#33BBE2'>high (Q30+)</font>, <font color='#666666'>moderate (Q20+)</font>, <font color='#E99E5B'>low (Q15+)</font>, <font color='#FF0000'>extremely low (0~Q14)</font> </li>";
    mFile << "<li> Move mouse over the base, it will show the quality value</li>";
    if(GlobalSettings::outputOriginalReads)
        mFile << "<li> Click on any row, the original read/pair will be displayed</li>";
    mFile << "<li> In first column, <i>d</i> means the edit distance of match, and --> means forward, <-- means reverse </li>";
    mFile << "<li> For pair-end sequencing, MutScan tries to merge each pair, and the overlapped bases will be assigned higher qualities </li>";
    mFile << "</ul></div>";
}

void HtmlReporter::printMutations() {
    // calculate the found mutation
    int found = 0;
    for(int i=0;i<mMutationList.size();i++){
        vector<Match*> matches = mMutationMatches[i];
        if(matches.size()>0){
            found++;
        }
    }
    // print menu
    int id = 0;
    if(!mInFrame){
        mFile<<"<div id='menu'><p>Found "<< found << " mutation";
        if(found>1)
            mFile<<"s";
        mFile<<":</p><ul>";
        for(int i=0;i<mMutationList.size();i++){
            vector<Match*> matches = mMutationMatches[i];
            if(matches.size()>0){
                id++;
                mFile<<"<li class='menu_item'><a href='#"<<mMutationList[i].mName<<"'> " << id << ", " << mMutationList[i].mName;
                mFile<< " (" << matches.size() << " reads support, " << Match::countUnique(matches) << " unique)" << "</a></li>";
            }
        }
        mFile<<"</ul></div>";
    }
    id=0;
    for(int i=0;i<mMutationList.size();i++){
        vector<Match*> matches = mMutationMatches[i];
        if(matches.size()>0){
            id++;
            printMutation(id, mMutationList[i], matches);
        }
    }
}

void HtmlReporter::printMutationsJS() {
    mFile << "\n<script type=\"text/javascript\">" << endl;
    mFile << "var data_break = [";
    int id=0;
    for(int i=0;i<mMutationList.size();i++){
        vector<Match*> matches = mMutationMatches[i];
        if(matches.size()>0){
            mFile << "\n[";
            for(int m=0; m<matches.size(); m++){
                mFile << "\n[";
                matches[m]->printJS(mFile, mMutationList[i].mLeft.length(), mMutationList[i].mCenter.length(), mMutationList[i].mRight.length());
                mFile << "],"; 
            }
            id++;
            mFile << "],";
        }
    }
    mFile << "];";
    mFile << "\nfor(var mutid=0;mutid<data_break.length;mutid++){";
    mFile << "\nfor(var matchid=0;matchid<data_break[mutid].length;matchid++){";
    mFile << "\nfor(var breakid=0;breakid<data_break[mutid][matchid].length;breakid++){";
    mFile << "\nvar target = 'b-' + mutid + '-' + matchid + '-' + breakid;";
    mFile << "\ndocument.getElementById(target).innerHTML = colorize(data_break[mutid][matchid][breakid][0], data_break[mutid][matchid][breakid][1]);";
    mFile << "\n}}}";

    mFile << "\nfunction makeColor(qual) { \
            if(qual >= 'I') \
                return '#78C6B9'; \
            if(qual >= '?') \
                return '#33BBE2'; \
            if(qual >= '5') \
                return '#666666'; \
            if(qual >= '0') \
                return '#E99E5B'; \
            else \
                return '#FF0000'; \
        }";

    mFile << "\nfunction colorize(seq, qual) { \
        \nvar str = ''; \
        \nfor(var i=0;i<seq.length;i++) { \
        \n    str += \"<a title='\" + qual[i] + \"'><font color='\" + makeColor(qual[i]) + \"'>\" + seq[i] + \"</font></a>\" \
        \n} \
        \nreturn str; \
    \n}";
    mFile << "\n</script>";
}

void HtmlReporter::printMutation(int id, Mutation& mutation, vector<Match*>& matches){
    mFile << "\n<div class='mutation_block'>";
    mFile << "<div class='mutation_head'><a name='" << mutation.mName << "'>";
    if(!mInFrame)
        mFile << id << ", ";
    mFile << mutation.mName<< " (" << matches.size() << " reads support, " << Match::countUnique(matches) << " unique)" ;
    mFile << "</a></div>";
    mFile << "<table>";
    mFile << "<tr class='header'>";
    mFile << "<td class='match_brief'>" << "ID_Distance_Strand" << "</td>";
    mFile << "<td>" << "" << "</td>";
    mFile << "<td>" << mutation.mLeft << "</td>";
    mFile << "<td>" << mutation.getCenterHtml() << "</td>";
    mFile << "<td>" << mutation.mRight << "</td>";
    mFile << "<td>" << "" << "</td>";
    mFile << "</tr>";
    for(int m=0; m<matches.size(); m++){
        long rowid = id*100000 + m;
        if(GlobalSettings::outputOriginalReads)
            mFile << "<tr onclick='toggle(" << rowid << ");'>";
        else
            mFile << "<tr>";
        mFile << "<td>";
        // for display alignment
        if(m+1<10)
            mFile<<"0";
        if(m+1<100)
            mFile<<"0";
        if(m+1<1000)
            mFile<<"0";
        mFile << m+1 << ", ";
        matches[m]->printHtmlTD(mFile, mutation.mLeft.length(), mutation.mCenter.length(), mutation.mRight.length(), id-1, m);
        mFile << "</tr>";
        // print a hidden row containing the full read
        if(GlobalSettings::outputOriginalReads){
            mFile << "<tr id='" << rowid << "' style='display:none;'>";
            mFile << "<td colspan='6'><xmp>";
            matches[m]->printReadsToFile(mFile);
            mFile << "</xmp></td>";
            mFile << "</tr>";
        }
    }
    mFile << "</table></div>";
}

void HtmlReporter::printHeader(){
    mFile << "<html><head><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\" />";
    mFile << "<title>MutScan report</title>";
    printJS();
    printCSS();
    mFile << "</head>";
    mFile << "<body><div id='container'>";
}

void HtmlReporter::printCSS(){
    if(mInFrame){
        mFile << "<link type='text/css' rel='stylesheet' charset='utf-8' href='../mutscan.css'/>";
    } else {
        mFile << "<style type=\"text/css\">";
        mFile << "td {border:1px solid #dddddd;padding-left:2px;padding-right:2px;font-size:10px;}";
        mFile << "table {border:1px solid #999999;padding:2x;border-collapse:collapse;}";
        mFile << "img {padding:30px;}";
        mFile << "#menu {font-family:Consolas, 'Liberation Mono', Menlo, Courier, monospace;}";
        mFile << "#menu a {color:#0366d6; font-size:18px;font-weight:600;line-height:28px;text-decoration:none;font-family:-apple-system, BlinkMacSystemFont, 'Segoe UI', Helvetica, Arial, sans-serif, 'Apple Color Emoji', 'Segoe UI Emoji', 'Segoe UI Symbol'}";
        mFile << "a:visited {color: #999999}";
        mFile << ".alignleft {text-align:left;}";
        mFile << ".alignright {text-align:right;}";
        mFile << ".header {color:#ffffff;padding:1px;height:20px;background:#000000;}";
        mFile << ".figuretitle {color:#996657;font-size:20px;padding:50px;}";
        mFile << "#container {text-align:center;padding:1px;font-family:Arail,'Liberation Mono', Menlo, Courier, monospace;}";
        mFile << "#menu {padding-top:10px;padding-bottom:10px;text-align:left;}";
        mFile << ".menu_item {text-align:left;padding-top:5px;font-size:18px;}";
        mFile << ".highlight {text-align:left;padding-top:30px;padding-bottom:30px;font-size:20px;line-height:35px;}";
        mFile << ".mutation_head {text-align:left;font-family:Arial;padding-top:20px;padding-bottom:5px;color:#999999}";
        mFile << ".mutation_block {}";
        mFile << ".match_brief {font-size:8px}";
        mFile << ".mutation_point {color:#FFCCAA}";
        mFile << "#helper {text-align:left;border:1px dotted #fafafa;color:#777777;}";
        mFile << "#footer {text-align:left;padding-left:10px;padding-top:20px;color:#777777;font-size:10px;}";
        mFile << "</style>";
    }
}

void HtmlReporter::printJS(){
    if(mInFrame){
        mFile << "<script type='text/javascript' src='../mutscan.js'></script>";
    } else {
        mFile << "\n<script type=\"text/javascript\">" << endl;
        mFile << "function toggle(targetid){ \n\
                    if (document.getElementById){ \n\
                        target=document.getElementById(targetid); \n\
                            if (target.style.display=='table-row'){ \n\
                                target.style.display='none'; \n\
                            } else { \n\
                                target.style.display='table-row'; \n\
                            } \n\
                    } \n\
                }";
        mFile << "function toggle_target_list(targetid){ \n\
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
        mFile << "</script>";
    }
}

const std::string getCurrentSystemTime()
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

void HtmlReporter::printFooter(){
    if(!mInFrame){
        mFile << "\n<div id='footer'> ";
        mFile << "<p>"<<command<<"</p>";
        printScanTargets();
        mFile << "MutScan " << MUTSCAN_VER << ", at " << getCurrentSystemTime() << " </div>";
    }
    mFile << "</div></body></html>";
}

void HtmlReporter::printScanTargets(){
    mFile << "\n<div id='targets'> ";
    mFile << "<p> scanned " << mMutationList.size() << " mutation spots...<input type='button' id='target_view_btn', onclick=toggle_target_list('target_list'); value='show'></input></p>";
    mFile << "<ul id='target_list' style='display:none'>";
    int id=0;
    for(int i=0;i<mMutationList.size();i++){
        id++;
        mFile<<"<li> " << id << ", " << mMutationList[i].mName << "</li>";
    }
    mFile << "</ul></div>";
}