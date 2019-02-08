#ifndef SYS_TIME_H_FOR_WINDOWS_
#define SYS_TIME_H_FOR_WINDOWS_

#ifdef _WIN32
#include <time.h>
#include <windows.h>
// From https://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows
#define CLOCK_MONOTONIC 0
int clock_gettime(int, struct timespec *spec)
{
    __int64 wintime;
    GetSystemTimeAsFileTime((FILETIME*)&wintime);
    wintime      -= 116444736000000000i64;           // 1601 to 1970
    spec->tv_sec  = wintime / 10000000i64;           // seconds
    spec->tv_nsec = wintime % 10000000i64 *100;      // nanoseconds
    return 0;
}
#else
#include <sys/time.h>
#endif

#endif  // SYS_TIME_H_FOR_WINDOWS_
