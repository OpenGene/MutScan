# MutScan
Detect important mutations by scanning FastQ files directly

# Build
```shell
# download Release, or clone from github:
git clone https://github.com/OpenGene/MutScan.git
cd MutScan
make
```

#Usage
```shell
usage: mutscan --read1=string --read2=string --mutation=string [options] ... 
options:
  -1, --read1       read1 file name (string)
  -2, --read2       read2 file name (string)
  -m, --mutation    mutation file name (string)
  -?, --help        print this message
```
The result, contains the detected mutations and their support reads, will be printed directly. You can use `>` to redirect output to a file, like:
```shell
mutscan --read1=string --read2=string --mutation=string > result.txt
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
A default CSV file contains important actionable cancer gene targets is already provided in `mutation/cancer.csv`
