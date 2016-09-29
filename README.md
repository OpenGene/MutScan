# MutScan
Detect important mutations by scanning FastQ files directly
* Ultra sensitive
* 20x faster than normal pipeline (i.e. BWA + Samtools + GATK/VarScan/Mutect)
* Very easy to use. Need nothing else. No alignment, no reference assembly, no variant call, no pileup...
* Beautiful HTML report

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
usage: mutscan -1 <read1_file_name> -2 <read2_file_name> -m <mutation_file_name> -h <html_report_filename>[options] ... 
options:
  -1, --read1       read1 file name (string)
  -2, --read2       optional, read2 file name (string)
  -m, --mutation    optional, mutation file name (string)
  -h, --html        optional, filename of html report, no html report if not specified (string)
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

# Mutation file
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
A default CSV file contains important actionable cancer gene targets is already provided in `mutation/cancer.csv`. If you want to use this mutation file directly, the argument `mutation_file_name` can be omitted:
```shell
mutscan -1 <read1_file_name> -2 <read2_file_name>
```

# HTML output
If `-h` or `--html` argument is given, then a HTML report will be generated, and written to the given filename. A sample report is given here:   

![image](https://github.com/OpenGene/MutScan/raw/master/testdata/sample_report.jpg)  
The color of each base indicates its quality, and the quality will be shown when mouse over.
