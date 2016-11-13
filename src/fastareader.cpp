
#include "fastareader.h"
#include "util.h"

// FastaReader.cpp

/* Constructor */
FastaReader::FastaReader(std::string faFile)
{
    // Set locale and disable stdio synchronization to improve iostream performance
    // http://www.drdobbs.com/the-standard-librarian-iostreams-and-std/184401305
    // http://stackoverflow.com/questions/5166263/how-to-get-iostream-to-perform-better
    setlocale(LC_ALL,"C");
    std::ios_base::sync_with_stdio(false);

    fastaFile = faFile;
    hasMoreNucleotides = false;
    currentSequence = "";
// verify that filename is NOT a directory
    if (is_directory(fastaFile)) {
        std::string error_msg = "There is a problem with the provided fasta file: \'";
        error_msg.append(fastaFile);
        error_msg.append("\' is a directory NOT a file...\n");
        throw std::invalid_argument(error_msg);
    }
    fastaFileStream.open( fastaFile.c_str(),std::ios::in);
    // verify that the file can be read
    if (!fastaFileStream.is_open()) {
        std::string msg = "There is a problem with the provided fasta file: could NOT read ";
        msg.append(fastaFile.c_str());
        msg.append("...\n");
        throw std::invalid_argument(msg);
    }
    currentSequence.reserve(100000);
}

/* Destructor */
FastaReader::~FastaReader()
{
    if (fastaFileStream.is_open()) {
        fastaFileStream.close();
    }
}


/* Set current fasta Sequence id  and description
  >id description
*/
void FastaReader::setFastaSequenceIdDescription()
{
    id = " ";
    description = " ";
    if (currentSequence.length() != 0) {
        std::size_t first_space = currentSequence.find(" ");
        if (first_space != std::string::npos) {
            id = currentSequence.substr(0,first_space);
            description = currentSequence.substr(first_space+1);
        } else {
            id = currentSequence;
        }
    }
}


/* hasNextSequence
   - read fasta file up to the next header of a Sequence (e.g. >accession ...)
 */
bool FastaReader::hasNextSequence()
{
    char c;
    std::string header;
    // read to next >
    while (fastaFileStream.get(c) && c != '>') {
        if (fastaFileStream.eof()) {
            break;
        }
    }
    if (fastaFileStream.eof()) {
        hasMoreNucleotides = false;
        return false;
    }
    for (header = ""; fastaFileStream.get(c) && !endOfLine(c) ;) {
        header += c;
    }
    hasMoreNucleotides = true;
    currentSequence = header;
    setFastaSequenceIdDescription();
    currentSequence = "";
    return true;
}

/* readSequence : read next fasta record*/
void FastaReader::readSequence()
{
    currentSequence.clear();
    while (hasMoreNucleotides) {
        FastaReader::readLine();
    }
}

/*  readLine
    - read a line of the fasta file into currentSequence
    - non alphabetical char are ignored (e.g space , tabs, ...)
    - no limit on the length of the line other than the maximum size of std::string
 */
bool FastaReader::readLine()
{
    std::string line = "";
    if('>' != fastaFileStream.peek()) {
        getline(fastaFileStream,line,'\n');
        currentSequence += str_keep_valid_sequence(line);
    }
    hasMoreNucleotides = !fastaFileStream.eof();
    if ('>' == fastaFileStream.peek()) {
        hasMoreNucleotides = false;
    } else if (fastaFileStream.eof()) {
        hasMoreNucleotides = false;
    } else {
        hasMoreNucleotides = true;
    }
    return hasMoreNucleotides;
}

/* endOfLine
   - check if a char of the fasta file is end of line (\r \n EOF or >)
 */
bool FastaReader::endOfLine(char c)
{
    bool eol = ( '\r' == c || '\n' == c || '>' == c);
    if  ('>' == c ) {
        fastaFileStream.putback(c);
        hasMoreNucleotides = false;
    }
    return eol;
}

/* print id and description of current fasta record */
void FastaReader::printCurrentFastaHeader(std::ostream &os)
{
    os << Id() << " " << Description()  << std::endl;
}

/* print sequence of current fasta record*/
void FastaReader::printCurrentSequence(std::ostream &os)
{
    os << currentSequence << std::endl;
}


/* SubSequence
   - get portion of the current sequence
   - if the starting  or ending position are not valid, an empty string is returned ("")
*/
std::string FastaReader::SubSequence(std::size_t start,std::size_t end)
{
    std::string subseq = "";
    std::size_t starting_pos  = start -1;
    std::size_t size  = end-start+1;
    std::size_t ending_pos  = starting_pos+size-1;
    if ( currentSequence.size() > 0 &&
         start != 0  &&
         ending_pos <= currentSequence.size()-1
       ) {
        subseq = currentSequence.substr(starting_pos,size);
    }
    return subseq;
}



