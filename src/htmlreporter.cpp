#include "htmlreporter.h"

HtmlReporter::HtmlReporter(string filename, vector<Mutation>& mutationList, vector<Match*> *mutationMatches){
    mMutationList = mutationList;
    mMutationMatches = mutationMatches;
    mFilename = filename;
    mFile.open(mFilename, ifstream::out);
}

HtmlReporter::~HtmlReporter(){
    mFile.close();
}

void HtmlReporter::run() {
    printHeader();
    mFile<<"Testhtml";
    printFooter();
}

void HtmlReporter::printHeader(){
    mFile << "<html><head><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\" />";
    mFile << "<title>MutScan report</title>";
    printCSS();
    mFile << "</head>";
    mFile << "<body><div id='container'>";
}

void HtmlReporter::printCSS(){
    mFile << "<style type=\"text/css\">";
    mFile << "td {border:1px solid #dddddd;color:#999999;text-align:center;padding-left:2px;padding-right:2px;font-size:8px;}";
    mFile << "table {border:1px solid #999999;padding:2x;border-collapse:collapse;}";
    mFile << ".finding_table td{padding:10px;border:1px solid #999999;border-collapse:collapse;font-size:16px;color:#666666;}";
    mFile << ".finding_table{width:95%;padding:30px;}";
    mFile << ".mutation_table td{padding:5px;font-size:12px;color:#666666;}";
    mFile << "img {padding:30px;}";
    mFile << "img {padding:30px;}";
    mFile << ".header {color:#669933;font-weight:bold;font-size:8px;padding:1px;}";
    mFile << ".odd {background:#eeeeee;}";
    mFile << ".cosmic {font-weight:bold;color:#FF6600;}";
    mFile << ".label {background:#666666;color:#FFFFFF;}";
    mFile << ".sampleid {color:#996657;font-size:8px;}";
    mFile << ".figuretitle {color:#996657;font-size:20px;padding:50px;}";
    mFile << "#container {width:100%;text-align:center;padding:1px;}";
    mFile << "#menu {width:100%;text-align:left;padding:10px;font-size:28px;line-height:55px;}";
    mFile << ".highlight {width:100%;text-align:left;padding-top:30px;padding-bottom:30px;font-size:20px;line-height:35px;}";
    mFile << "</style>";
}

void HtmlReporter::printFooter(){
    mFile << "</div></body></html>";
}