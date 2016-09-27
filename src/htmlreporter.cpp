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
    printMutations();
    printFooter();
}

void HtmlReporter::printMutations() {
    for(int i=0;i<mMutationList.size();i++){
        vector<Match*> matches = mMutationMatches[i];
        if(matches.size()>0){
            printMutation(mMutationList[i], matches);
        }
    }
}

void HtmlReporter::printMutation(Mutation& mutation, vector<Match*>& matches){
    mFile << "<table>";
    mFile << "<tr class='header'>";
    mFile << "<td>" << "" << "</td>";
    mFile << "<td>" << "" << "</td>";
    mFile << "<td>" << mutation.mLeft << "</td>";
    mFile << "<td>" << mutation.mCenter << "</td>";
    mFile << "<td>" << mutation.mRight << "</td>";
    mFile << "<td>" << "" << "</td>";
    mFile << "</tr>";
    for(int m=0; m<matches.size(); m++){
        mFile << "<tr>";
        mFile << "<td>" << m+1 << ", ";
        matches[m]->printHtmlTD(mFile, mutation.mLeft.length(), mutation.mCenter.length(), mutation.mRight.length());
        mFile << "</tr>";
    }
    mFile << "</table>";
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
    mFile << "td {border:1px solid #dddddd;padding-left:2px;padding-right:2px;font-size:10px;}";
    mFile << "table {border:1px solid #999999;padding:2x;border-collapse:collapse;}";
    mFile << "img {padding:30px;}";
    mFile << ".alignleft {text-align:left;}";
    mFile << ".alignright {text-align:right;}";
    mFile << ".header {color:#ffffff;padding:1px;height:20px;background:#000000;}";
    mFile << ".figuretitle {color:#996657;font-size:20px;padding:50px;}";
    mFile << "#container {width:100%;text-align:center;padding:1px;}";
    mFile << "#menu {width:100%;text-align:left;padding:10px;font-size:28px;line-height:55px;}";
    mFile << ".highlight {width:100%;text-align:left;padding-top:30px;padding-bottom:30px;font-size:20px;line-height:35px;}";
    mFile << "</style>";
}

void HtmlReporter::printFooter(){
    mFile << "</div></body></html>";
}