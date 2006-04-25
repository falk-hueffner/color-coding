#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>

#define WHITESPACE " \f\r\t\v"

double timestamp();
std::string trim(const std::string& s);
std::vector<std::string> split(const std::string& s);

inline unsigned isolate_lowest_bit(unsigned x) {
    return x & -x;
}

unsigned bits_needed(std::size_t max);
unsigned peek_bits(const unsigned char* p, unsigned word_size, std::size_t offset);
void poke_bits(unsigned char* p, std::size_t word_size, std::size_t offset, std::size_t val);

#endif // UTIL_H
