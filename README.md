# MutScan
Detect important mutations by scanning FastQ files directly
* Ultra sensitive
* 20X+ faster than normal pipeline (i.e. BWA + Samtools + GATK/VarScan/Mutect)
* Very easy to use. Need nothing else. No alignment, no reference assembly, no variant call, no pileup...
* Beautiful HTML report
* Multi-threading support
* Support both single-end and pair-end data
* For pair-end data, MutScan will try to merge each pair, and do quality adjustment and error correction

# Sample report
http://opengene.org/MutScan/report.html

# Download
```shell
# download use http
https://github.com/OpenGene/MutScan/archive/master.zip

# or download use git
git clone https://github.com/OpenGene/MutScan.git
```

# Build
```shell
cd MutScan
make
```

#Usage
```shell
usage: mutscan -1 <read1_file> -2 <read2_file> -m <mutation_file> -h <html_report_file> -t <thread> 
options:
  -1, --read1       read1 file name (string)
  -2, --read2       read2 file name (string [=])
  -m, --mutation    mutation file name, can be a CSV format or a VCF format (string [=])
  -r, --ref         reference fasta file name (only needed when mutation file is a VCF) (string [=])
  -h, --html        filename of html report, no html report if not specified (string [=])
  -t, --thread      worker thread number, default is 4 (int [=4])
  -k, --mark        when mutation file is a vcf file, --mark means only process the records with FILTER column is M
  -?, --help        print this message
```
The plain text result, contains the detected mutations and their support reads, will be printed directly. You can use `>` to redirect output to a file, like:
```shell
mutscan -1 <read1_file_name> -2 <read2_file_name> -m <mutation_file_name> > result.txt
```
And you can make a HTML file report with `-h` argument, like:
```
mutscan -1 <read1_file_name> -2 <read2_file_name> -m <mutation_file_name> -h report.html
```
## single-end and pair-end
For single-end sequencing data, `-2` argument is omitted:
```
mutscan -1 <read1_file_name> -m <mutation_file_name>
```
## multi-threading
`-t` argument specify how many worker threads will be launched. The default thread number is `4`. Suggest to use a number less than the CPU cores of your system.

# Mutation file
Mutation file, specified by `-m`, can be a `CSV file`, or a `VCF file`. If this file is not specified, MutScan will use the built-in default mutation file with about 50 cancer related locuses.
## CSV-format mutation file
A CSV file with columns of `name`, `left_seq_of_mutation_point`, `mutation_seq` and `right_seq_of_mutation_point`
```csv
#name, left_seq_of_mutation_point, mutation_seq, right_seq_of_mutation_point
NRAS-neg-1-115258748-2-c.34G>A-p.G12S-COSM563, GGATTGTCAGTGCGCTTTTCCCAACACCAC, T, TGCTCCAACCACCACCAGTTTGTACTCAGT
NRAS-neg-1-115252203-2-c.437C>T-p.A146V-COSM4170228, TGAAAGCTGTACCATACCTGTCTGGTCTTG, A, CTGAGGTTTCAATGAATGGAATCCCGTAAC
BRAF-neg-7-140453136-15-c.1799T>A -V600E-COSM476, AACTGATGGGACCCACTCCATCGAGATTTC, T, CTGTAGCTAGACCAAAATCACCTATTTTTA
EGFR-pos-7-55241677-18-c.2125G>A-p.E709K-COSM12988, CCCAACCAAGCTCTCTTGAGGATCTTGAAG, A, AAACTGAATTCAAAAAGATCAAAGTGCTGG
EGFR-pos-7-55241707-18-c.2155G>A-p.G719S-COSM6252, GAAACTGAATTCAAAAAGATCAAAGTGCTG, A, GCTCCGGTGCGTTCGGCACGGTGTATAAGG
EGFR-pos-7-55241707-18-c.2155G>T-p.G719C-COSM6253, GAAACTGAATTCAAAAAGATCAAAGTGCTG, T, GCTCCGGTGCGTTCGGCACGGTGTATAAGG
```
## VCF-format mutation file
A standard VCF can be used as a mutation file, with file extension `.vcf` or `.VCF`. 
* if the VCF file is smaller than 100 records, all records can be scanned
* if the VCF file has more than 100 records, you should add `--mark` in the command line, and then mark the wanted records with the `FILTER` column marked `M`. For example (note the M in the FILTER column):
```
#CHROM   POS     ID          REF ALT QUAL  FILTER  INFO
1        69224   COSM3677745 A   C   .     M       This record will be scanned
1        880950  COSM3493111 G   A   .     .       This record will be skipped
```
## Use default mutations
A default CSV file contains important actionable cancer gene targets is already provided in `mutation/cancer.csv`. If you want to use this mutation file directly, the argument `mutation_file_name` can be omitted:
```shell
mutscan -1 <read1_file_name> -2 <read2_file_name>
```

# HTML output
If `-h` or `--html` argument is given, then a HTML report will be generated, and written to the given filename. A sample report is given here:   

![image](https://github.com/OpenGene/MutScan/raw/master/testdata/sample_report.jpg)  
* The color of each base indicates its quality, and the quality will be shown when mouse over.
* Click on any row, the original read/pair will be displayed
* In first column, d means the edit distance of match, and --> means forward, <-- means reverse 
