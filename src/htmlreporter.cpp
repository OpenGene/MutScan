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
    // calculate the found mutation
    int found = 0;
    for(int i=0;i<mMutationList.size();i++){
        vector<Match*> matches = mMutationMatches[i];
        if(matches.size()>0){
            found++;
        }
    }
    // print menu
    mFile<<"<div id='menu'><p>Found "<< found << " mutations:</p><ul>";
    int id = 0;
    for(int i=0;i<mMutationList.size();i++){
        vector<Match*> matches = mMutationMatches[i];
        if(matches.size()>0){
            id++;
            mFile<<"<li class='menu_item'><a href='#"<<mMutationList[i].mName<<"'> " << id << ", " << mMutationList[i].mName << " (" << matches.size() << " reads support)" << "</a></li>";
        }
    }
    mFile<<"</ul></div>";
    id=0;
    for(int i=0;i<mMutationList.size();i++){
        vector<Match*> matches = mMutationMatches[i];
        if(matches.size()>0){
            id++;
            printMutation(id, mMutationList[i], matches);
        }
    }
}

void HtmlReporter::printMutation(int id, Mutation& mutation, vector<Match*>& matches){
    mFile << "<div class='mutation_block'>";
    mFile << "<div class='mutation_head'><a name='" << mutation.mName << "'>";
    mFile << id << ", " << mutation.mName<< " (" << matches.size() << " reads support)" ;
    mFile << "</a></div>";
    mFile << "<table>";
    mFile << "<tr class='header'>";
    mFile << "<td class='match_brief'>" << "Distance_Strand" << "</td>";
    mFile << "<td>" << "" << "</td>";
    mFile << "<td>" << mutation.mLeft << "</td>";
    mFile << "<td>" << mutation.mCenter << "</td>";
    mFile << "<td>" << mutation.mRight << "</td>";
    mFile << "<td>" << "" << "</td>";
    mFile << "</tr>";
    for(int m=0; m<matches.size(); m++){
        mFile << "<tr>";
        mFile << "<td>";
        // for display alignment
        if(m+1<10)
            mFile<<"0";
        mFile << m+1 << ", ";
        matches[m]->printHtmlTD(mFile, mutation.mLeft.length(), mutation.mCenter.length(), mutation.mRight.length());
        mFile << "</tr>";
    }
    mFile << "</table></div>";
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
    mFile << "#container {text-align:center;padding:1px;font-family:Arial;}";
    mFile << "#menu {padding-top:10px;padding-bottom:10px;text-align:left;}";
    mFile << ".menu_item {text-align:left;padding-top:5px;font-size:18px;}";
    mFile << ".highlight {text-align:left;padding-top:30px;padding-bottom:30px;font-size:20px;line-height:35px;}";
    mFile << ".mutation_head {text-align:left;color:#0092FF;font-family:Arial;padding-top:20px;padding-bottom:5px;}";
    mFile << ".mutation_block {}";
    mFile << ".match_brief {font-size:8px}";
    mFile << "</style>";
}

void HtmlReporter::printFooter(){
    mFile << "</div></body></html>";
}