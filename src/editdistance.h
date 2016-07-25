#ifndef ___EDITDISTANCE__H__
#define ___EDITDISTANCE__H__

#include <stdint.h>

// struct PatternMap {
//     uint64_t p_[256][4];
//     unsigned int tmax_;
//     unsigned int tlen_;
// };

unsigned int edit_distance(const int64_t *a, const unsigned int asize, const int64_t *b, const unsigned int bsize);
// void create_patternmap(struct PatternMap *pm, const int64_t *a, const unsigned int size);
// unsigned int edit_distance_by_patternmap(struct PatternMap *mp, const int64_t *b, const unsigned int size);

unsigned int edit_distance(const char *str1, size_t const size1, const char *str2, size_t const size2);


#endif
