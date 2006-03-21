#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>

#define WHITESPACE " \f\r\t\v"

double timestamp();
std::string trim(const std::string& s);
std::vector<std::string> split(const std::string& s);

#endif // UTIL_H
