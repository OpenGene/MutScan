#include "overlap.h"
#include "editdistance.h"
#include "math.h"

Overlap::Overlap(int offset, int overlapLen, int distance){
    mOffset = offset;
    mOverlapLen = overlapLen;
    mDistance = distance;
    mOverlapped = overlapLen>0;
}

Overlap Overlap::fit(Sequence r1, Sequence r2){
    int len1 = r1.length();
    int len2 = r2.length();
    Sequence reverse_r2 = ~r2;

    bool overlapped = false;
    int overlap_len = 0;
    int offset = 0;
    int distance = 0;
    // a match of less than 10 is considered as unconfident
    while (offset < len1-10 && overlapped==false){
        // the overlap length of r1 & r2 when r2 is move right for offset
        overlap_len = min(len1-offset, len2);

        distance = edit_distance(r1.mStr.substr(offset, overlap_len), reverse_r2.mStr.substr(0, overlap_len));
        float threshold = min(3.0, overlap_len/10.0);
        if (distance <= threshold){
            // now we find a good candidate
            // we verify it by moving r2 one more base to see if the distance is getting longer
            // if yes, then current is the best match, otherwise it's not
            while (offset < len1-10){
                int next_offset = offset + 1;
                int next_overlap_len = min(len1-next_offset, len2);
                int next_distance = edit_distance(r1.mStr.substr(next_offset, next_overlap_len), reverse_r2.mStr.substr(0, next_overlap_len));
                if (distance <= next_distance){
                    overlapped = true;
                    break;
                }
                else{
                    offset = next_offset;
                    distance = next_distance;
                    overlap_len = next_overlap_len;
                }
            }
            break;
        }
        else
            offset += max(1, (distance - (int)ceil(threshold))/2 );
    }

    if (overlapped && offset == 0){
        // check if distance can get smaller if offset goes negative
        // this only happens when insert DNA is shorter than sequencing read length, and some adapter/primer is sequenced but not trimmed cleanly
        // we go reversely
        while (offset > -(len2-10)){
            // the overlap length of r1 & r2 when r2 is move right for offset
            overlap_len = min(len1,  len2- abs(offset));
            distance = edit_distance(r1.mStr.substr(0, overlap_len), reverse_r2.mStr.substr(-offset, overlap_len));
            float threshold = min(3.0, overlap_len/10.0);
            if (distance <= threshold){
                while(offset > -(len2-10)){
                    int next_offset = offset - 1;
                    int next_overlap_len = min(len1,  len2- abs(next_offset));
                    int next_distance = edit_distance(r1.mStr.substr(0, next_overlap_len), reverse_r2.mStr.substr(-next_offset, next_overlap_len));
                    if (distance <= next_distance)
                        return Overlap(offset, overlap_len, distance);
                    else {
                        distance = next_distance;
                        overlap_len = next_overlap_len;
                        offset = next_offset;
                    }
                }
            }
            else
                offset -= max(1, (distance - (int)ceil(threshold))/2 );
        }
    }
    else if(overlapped) {
        return Overlap(offset, overlap_len, distance);
    }

    return Overlap(0,0,0);
}
bool Overlap::test(){
    Sequence r1[5] = {
        Sequence("TTTGCAGGCACCTACCACTGTACCTGTCTAATTTTTCTTCTGCCCTTTTTTTTTTTTTTTTTTTTTTTTTGGGGTAGAGACGAGGCCTTGCTATGTAGCCCTTGCTGGTCTCAAACTCCTCGCCTCAAGTGATCCTCCTGCCTCGGCCTCC"),
        Sequence("CCCTATGTCTACAAAACATCAGAAAATTAGGGTGTGGTGGCTCATGCCTATAGTCATAGCTACATAGGAGGCTGAGGCAGGAGGATCGCTTGAGGGCAGGAGGATCACTCGAGCTCTGAAGGTCAACGCTGCAGTGAGCTATGATCGTGCC"),
        Sequence("TAGAGGGCTCAGATGCATTCCTTTTTAGCAGTGCTCTTATTTGGCATTGGTGGTGCTGTTTCTGTTGACCACTCCCAGAGTCTCTGGATGTTTTGTTATTCCTTTACCTCCCTAGCCTCTCCTTGGGGTTTCTTTGCAGGCTCTTGCTCTC"),
        Sequence("CCTGGGTAGCTGGGATACAGGCGCCCGCCACCACGCCCGGCTAATTTTGTATTTTTAGTAGAGACGAGGTTTCACCACATTGGCCAGGCTGGTCTCAAACTCCTGACCTCAGGTGATCTGCCTGCCTCAGCCTCCTAGAGTGCTGGG"),
        Sequence("GTTCCTTTTAACATAGAAAGCAGCTAATTTTCCTATTCAAAAAATGGAGCTCTATTAAAAGATAAAACAGCAGCTTAGCTCTAGGTAAAGTGATCCATGCGGTTCTTCTTCTTTTTTTTGTTTTGAGATGGACTCTCGCTCTGTCACCCA")
    };
    Sequence r2[5] = {
        Sequence("CATGGTGGCTCATGCCTGTAATCCCAGTGGTTTGGGAGGCCGAGGCAGGAGGATCACTTGAGGCGAGGAGTTTGAGACCAGCAAGGGCTACATAGCAAGGCCTCGTCTCTACCCCAAAAAAAAAAAAAAAAAAAAAAAAAGGGCAGAAGAA"),
        Sequence("AGTGCAGTGGCACGATCATAGCTCACTGCAGCGTTGACCTTCAGAGCTCGAGTGATCCTCCTGCCCTCAAGCGATCCTCCTGCCTCAGCCTCCTATGTAGCTATGACTATAGGCATGAGCCACCACACCCTAATTTTCTGATGTTTTGTAG"),
        Sequence("CTGGAGATAAACACCTAGCAGTCATGAGACAAAGCTCTGCAATGCTTGTATTTATGGGATACAAGAGAGAGCAAGAGCCTGCAAAGAAACCCCAAGGAGAGGCTAGGGAGGTAAAGGAATAACAAAACATCCAGAGACACTGGGAGTGGTC"),
        Sequence("CCCAGCACTCTAGGAGGCTGAGGCAGGCAGATCACCTGAGGTCAGGAGTTTGAGACCAGCCTGGCCAATGTGGTGAAACCTCGTCTCTACTAAAAATACAAAATTAGCCGGGCGTGGTGGCGGGCGCCTGTAATCCCAGCTACCCAGC"),
        Sequence("TGGGTGACAGAGCGAGAGTCCATCTCAAAACAAAAAAAAGAAGAAGAACCGCACTGGATCACTTTACCTCAGAGCTAAGCTGCTGTTTTATCTTTTAATAGAGCTCCATTTTTTGAATAGGAAAATTAGCTGCTTTCTATGTTAAAAGGAA")
    };
    Overlap overlap[5] = {
        Overlap(34L, 117L, 0L),
        Overlap(8L, 143L, 0L),
        Overlap(66L, 85L, 1L),
        Overlap(-1, 147, 2L),
        Overlap(0, 0, 0)
    };
    for (int i=0;i<5;i++){
        Overlap fit = Overlap::fit(r1[i], r2[i]);
        if (fit.mOffset!=overlap[i].mOffset || fit.mOverlapLen!=overlap[i].mOverlapLen || fit.mDistance!=overlap[i].mDistance){
            cout<<"Fail in Overlap::fit() with sequence"<<endl;
            cout<<r1[i].mStr<<endl;
            cout<<r2[i].mStr<<endl;
            return false;
        }
    }
    return true;
}