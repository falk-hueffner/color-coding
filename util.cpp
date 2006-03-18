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

std::string strip_comment(const std::string& s) {
    return std::string(s, s.find('#'));
}
