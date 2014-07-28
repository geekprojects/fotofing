#ifndef __FOTOFING_UTILS_H_
#define __FOTOFING_UTILS_H_

#include <string>

#include <time.h>

time_t tm2time(const struct tm *src);
std::string timeToString(time_t t);
std::string getHostName();

#endif
