#include <string>

#include <time.h>

#ifdef __unix__
#include <unistd.h>
#include <sys/times.h>
#endif

#include "util.h"

double timestamp() {
#ifdef __unix__
    struct tms buf;
    times(&buf);
    return (double) buf.tms_utime / sysconf(_SC_CLK_TCK);    
#else
    return clock() / CLOCKS_PER_SEC;
#endif
}

std::string trim(const std::string& s) {
    std::string::size_type b = s.find_first_not_of(WHITESPACE);
    if (b == std::string::npos)
	return "";
    std::string::size_type e = s.find_last_not_of(WHITESPACE);
    return std::string(s, b, e - b + 1);
}

std::vector<std::string> split(const std::string& s) {
    std::vector<std::string> result;
    std::string::size_type left = s.find_first_not_of(WHITESPACE);
    std::string::size_type right = s.find_first_of(WHITESPACE, left);
    while (left < right) {
	result.push_back(s.substr(left, right - left));
	left = s.find_first_not_of(WHITESPACE, right);
	right = s.find_first_of(WHITESPACE, left);
    }
    return result;
}

unsigned bits_needed(std::size_t max) {
    unsigned bits = 0;
    while (max) {
	max >>= 1;
	++bits;
    }
    return bits;
}

unsigned peek_bits(const unsigned char* p, unsigned word_size, std::size_t offset) {
    //assert(word_size <= 8);
    offset *= word_size;
    unsigned mask = (1 << word_size) - 1;
    unsigned r = offset % 8;
    unsigned lo = p[offset / 8];
    unsigned hi = p[(offset + word_size - 1) / 8];
    lo >>= r;
    hi <<= (8 - r);
    return (lo | hi) & mask;
}

void poke_bits(unsigned char* p, std::size_t word_size, std::size_t offset, std::size_t val) {
    //assert(word_size <= 8);
    offset *= word_size;
    unsigned r = offset % 8;
    unsigned mask = (1 << word_size) - 1;
    unsigned lo = p[offset / 8];
    unsigned hi = p[(offset + word_size - 1) / 8];
    lo &= ~(mask << r);
    hi &= ~(mask >> (8 - r));
    lo |= (val << r);
    hi |= (val >> (8 - r));
    // since possibly &hi == &lo, we need to write back hi first.
    p[(offset + word_size - 1) / 8] = hi;
    p[offset / 8] = lo;
}
