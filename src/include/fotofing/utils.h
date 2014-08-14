#ifndef __FOTOFING_UTILS_H_
#define __FOTOFING_UTILS_H_

#include <string>

#include <time.h>

time_t tm2time(const struct tm *src);
std::string timeToString(time_t t, bool time);
std::string getHostName();

#endif
