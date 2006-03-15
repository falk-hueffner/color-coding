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
