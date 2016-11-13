#ifndef _FASTA_READER_HPP_
#define _FASTA_READER_HPP_

// includes
#include <cctype>
#include <clocale>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

/**
 * Class FastaReader
 *
 * @brief Fasta file reader.
 *
 * Fasta file reader. Sequences (fasta record) are read one at a time
 * to limit memory usage. non alphabetical characters present in the
 * sequences are ignored (e.g space , tabs, ...) .
 *
 *
 * <b>Fasta file format:</b>
 *
 *   >id description<br>
 *   ACGTN
 *
 * <b>LIMITATION:</b>
 *
 *  max size of a sequence is the maximum size of std::string
 *  (std::size_t)                          <br>
 *
 *  - 4 611 686 018 427 387 897 nucleotides Linux         x86_64 g++ 4.6.3
 *  - 4 611 686 018 427 387 897 nucleotides OSX           x86_64 g++ 4.8.3
 *  -             4 294 967 294 nucleotides Windows 7     i686   Visual Studio 2010
 *  -             4 294 967 294 nucleotides Windows Vista i686   Visual Studio 2008
 *  -             4 294 967 289 nucleotides Linux         x86_64 g++ 3.4.6
 *  -             1 073 741 820 nucleotides Linux         i586   g++ 4.2.4
 *
 * <b>USAGE:</b> <br>
 *  <p>
 *  \#include "FastaReader.hpp"               <br>
 *                                            <br>
 *  FastaReader fr("file.fasta");             <br>
 *  while(fr.hasNextSequence())               <br>
 *  {                                         <br>
 *      fr.readSequence();        <br>
 *      std::string seq = fr.Sequence();        <br>
 *      // sequence manipulation              <br>
 *  }                                         <br>
 * </p>
 *
 * @author David Laperriere dlaperriere@outlook.com
 * @version 1.0e
 *
 *
 */
class FastaReader
{
public:


    /**
     * Contructor
     *
     * @param fastaFile  name of a fasta file
     *
     * @throw invalid_argument exception if the filename is a directory.
     * @throw invalid_argument exception if the file can not be opened.
     */
    FastaReader(std::string fastaFile);

    /**
     * Destructor
     *
     */
    ~FastaReader();

    /**
     * Read fasta file up to the next header of a record (e.g. >id description)
     *
     * @return true if a fasta record is found
     */
    bool hasNextSequence();


    /**
     * Read next sequence/fasta record.
     *
     *  - sequence is stored in 'currentSequence'
     *  - fasta record id is stored in 'id'
     *  - fasta record description is stored in 'description'
     */
    void readSequence();


    /**
     * @return id from the current fasta record (e.g. >id description)
     */
    std::string Id()
    {
        return id;
    }


    /**
     * @return description from the current fasta record (e.g. >id description)
     */
    std::string Description()
    {
        return description;
    }

    /**
     * @return lentgh of the current sequence
     */
    std::size_t SequenceLength()
    {
        return currentSequence.length();
    }

    /**
     * @return maximum lentgh a sequence can have
     */
    std::size_t MaxLength()
    {
        return currentSequence.max_size();
    }


    /**
     * Get portion of the current sequence.
     *
     * note: if the starting  or ending position are not valid,
     *       an empty string is returned ("")
     *
     * @param start starting position (1-based)
     * @param end ending position (1-based)
     * @return subsequence [start-end]
     */
    std::string SubSequence(std::size_t start,std::size_t end);


    /**
     * @return current sequence
     */
    std::string Sequence()
    {
        return currentSequence;
    }

    /**
     * Print header of the current fasta record (id description)
     *
     * @param os  output stream (default console)
     */
    void printCurrentFastaHeader(std::ostream &os=std::cout);

    /**
     * Print sequence of the current fasta record (
     *
     * @param os  output stream (default console)
     */
    void printCurrentSequence(std::ostream &os=std::cout);


private:

    /**
     * Read a line of the fasta file into currentSequence.
     *
     *  - non alphabetical char are ignored (e.g space , tabs, ...)
     *  - line can have has many char as a std::string can contain
     *  - line ends at \\r or \\n EOF or >
     *
     * @return true if there is more nucleotides to be read.
     * @see endOfLine(char c)
     */
    bool readLine();

    /**
     * Check if a char of the fasta file is end of line (\\r \\n EOF or >)
     *
     * @param c char to check
     * @return true if c mark the end of a line
     */
    bool endOfLine(char c);

    /**
     * Set id  and description from current fasta record
     * header (>id description).
     */
    void setFastaSequenceIdDescription();

    /**
     * Name of the fasta file.
     *
     * @see FastaReader(std::string fastaFile)
     */
    std::string fastaFile;

    /**
     * Sequence of the current fasta record.
     */
    std::string currentSequence;


    /**
     * id ofcurrent fasta record. (>id description)
     *
     * @see setFastaSequenceIdDescription()
     * @see getId()
     */
    std::string id ;

    /**
     * Description of the current fasta record. (>id description)
     *
     * @see setFastaSequenceIdDescription()
     * @see getDescription()
     */
    std::string description;

    /**
     * Input file stream of the fasta file.
     *
     * @see FastaReader(std::string fastaFile)
     * @see ~FastaReader()
     */
    std::ifstream fastaFileStream;

    /**
     * Internal state set to true if the fasta file has more sequence/fasta record.
     *
     * @see FastaReader(std::string fastaFile)
     * @see endOfLine(char c)
     * @see readLine()
     */
    bool hasMoreNucleotides;

};


#endif //_FASTA_READER_HPP_

