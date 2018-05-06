#ifndef MINMAX_H_
#define MINMAX_H_

inline int64_t min_int64(int64_t a, int64_t b)
{
    return a < b ? a : b;
}

inline int64_t max_int64(int64_t a, int64_t b)
{
    return a > b ? a : b;
}

#endif  // MINMAX_H_
