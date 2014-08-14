
#include <unistd.h>
#include <limits.h>

#include <fotofing/utils.h>

using namespace std;

time_t tm2time(const struct tm *src)
{
    struct tm tmp;

    tmp = *src;
    return timegm(&tmp) - src->tm_gmtoff;
}

string timeToString(time_t t, bool time)
{
    struct tm tm;
    localtime_r(&t, &tm);
    char buf[128];
    const char* fmt;

    if (time)
    {
        fmt = "%x %X";
    }
    else
    {
        fmt = "%x";
    }

    strftime(buf, 128, fmt, &tm);
    return string(buf);
}

string getHostName()
{
    int res;
    char hostname[HOST_NAME_MAX + 1];
    res = gethostname(hostname, HOST_NAME_MAX);
    if (res)
    {
        return "unknown-host";
    }
    return string(hostname);
}

