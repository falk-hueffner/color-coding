#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>

#define WHITESPACE " \f\r\t\v"

#if defined __GNUC__
# define GNUC_PREREQ(maj, min)						\
    ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
# define GNUC_PREREQ(maj, min) 0
#endif

double timestamp();
std::string trim(const std::string& s);
std::vector<std::string> split(const std::string& s);

inline unsigned isolate_highest_bit(unsigned x) {
#if GNUC_PREREQ(3, 4)
    return 1 << (31 - __builtin_clz(x));
#else
    x |= x >>  1;
    x |= x >>  2;
    x |= x >>  4;
    x |= x >>  8;
    x |= x >> 16;
    return (x - 1) << 1;
#endif
}

inline unsigned isolate_lowest_bit(unsigned x) {
    return x & -x;
}

unsigned bits_needed(std::size_t max);
unsigned peek_bits(const unsigned char* p, unsigned word_size, std::size_t offset);
void poke_bits(unsigned char* p, std::size_t word_size, std::size_t offset, std::size_t val);

#endif // UTIL_H
