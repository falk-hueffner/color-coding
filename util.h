#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>
#include <bitset>

#define COMPILE_TIME_ASSERT(expr)	extern char UNIQUE_NAME[(expr) ? 1 : -1]
#define UNIQUE_NAME			MAKE_NAME(__LINE__)
#define MAKE_NAME(line)			MAKE_NAME2(line)
#define MAKE_NAME2(line)		constraint_ ## line

#define WHITESPACE " \f\r\t\v"

double timestamp();
std::string trim(const std::string& s);
std::vector<std::string> split(const std::string& s);

template <typename T>
inline T isolate_lowest_bit(T x) {
    return x & -x;
}

template <typename T>
inline unsigned popcount(T x) {
    return std::bitset<std::numeric_limits<T>::digits>(x).count();
}

unsigned bits_needed(std::size_t max);
unsigned peek_bits(const unsigned char* p, unsigned word_size, std::size_t offset);
void poke_bits(unsigned char* p, std::size_t word_size, std::size_t offset, std::size_t val);

#endif // UTIL_H
