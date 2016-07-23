#include "fastqreader.h"
#include "util.h"

FastqReader::FastqReader(string filename){
	mFilename = filename;
	mZipFile = NULL;
	mZipped = false;
	mHasQuality = true;
	init();
}

FastqReader::~FastqReader(){
	if (mZipped){
		if (mZipFile){
			gzclose(mZipFile);
			mZipFile = NULL;
		}
	}
	else {
		if (mFile.is_open()){
			mFile.close();
		}
	}
}

void FastqReader::init(){
	if (isZipFastq(mFilename)){
		mZipFile = gzopen(mFilename.c_str(), "r");
		mZipped = true;
		Read* r = read();

		//test if it has quality line or not (fastq/fasta)
		if (r->mQuality[0] == '@')
			mHasQuality = false;
		delete r;
		gzrewind(mZipFile);
	}
	else if (isFastq(mFilename)){
		mFile.open(mFilename, ifstream::in);
		mZipped = false;
	}
}

Read* FastqReader::read(){
	const int maxLine = 1000;
	if (mZipped){
		if (mZipFile == NULL)
			return NULL;

		char line[maxLine];

		if(!gzgets(mZipFile, line, maxLine))return NULL;
		string name(line);

		if (!gzgets(mZipFile, line, maxLine))return NULL;
		string sequence(line);

		if (!gzgets(mZipFile, line, maxLine))return NULL;
		string strand(line);

		if (mHasQuality){
			if (!gzgets(mZipFile, line, maxLine))return NULL;
			string quality(line);
			Read* read = new Read(name, sequence, strand, quality);
			return read;
		}
		else {
			Read* read = new Read(name, sequence, strand);
			return read;
		}

	}
	return NULL;
}

bool FastqReader::isZipFastq(string filename) {
	if (ends_with(filename, ".fastq.gz"))
		return true;
	else if (ends_with(filename, ".fq.gz"))
		return true;
	else if (ends_with(filename, ".fasta.gz"))
		return true;
	else if (ends_with(filename, ".fa.gz"))
		return true;
	else
		return false;
}

bool FastqReader::isFastq(string filename) {
	if (ends_with(filename, ".fastq"))
		return true;
	else if (ends_with(filename, ".fq"))
		return true;
	else if (ends_with(filename, ".fasta"))
		return true;
	else if (ends_with(filename, ".fa"))
		return true;
	else
		return false;
}

bool FastqReader::isZipped(){
	return mZipped;
}