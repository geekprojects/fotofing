
#include <fotofing/utils.h>

using namespace std;

time_t tm2time(const struct tm *src)
{
    struct tm tmp;

    tmp = *src;
    return timegm(&tmp) - src->tm_gmtoff;
}

string timeToString(time_t t)
{
    struct tm tm;
    localtime_r(&t, &tm);
    char buf[64];
    strftime(buf, 64, "%x", &tm);
    return string(buf);
}

