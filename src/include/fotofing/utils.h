#ifndef __FOTOFING_UTILS_H_
#define __FOTOFING_UTILS_H_

#include <string>

#include <time.h>

class ProgressListener
{
 public:
    virtual void updateProgress(int progress, int total, std::string message) = 0;
};

time_t tm2time(const struct tm *src);
std::string timeToString(time_t t, bool time);
std::string getHostName();

#endif
