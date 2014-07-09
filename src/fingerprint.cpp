
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

#include <string>
#include <map>

#include <beyond/surface.h>

#include "sha1.h"

#include "fingerprint.h"

using namespace std;

string fingerprint(string path)
{
    size_t pos = path.rfind('/');
    string file = path;
    if (pos != path.npos)
    {
        file = path.substr(pos + 1);
    }

    Surface* surface = Surface::loadJPEG(path.c_str());

    float stepX = (float)surface->getWidth() / (float)FINGERPRINT_DIVIDE;
    float stepY = (float)surface->getHeight() / (float)FINGERPRINT_DIVIDE;
    int stepXi = (int)round(stepX);
    int stepYi = (int)round(stepY);
    int blockCount = (stepXi * stepYi);

    Surface* fingerprintSurface = new Surface(FINGERPRINT_DIVIDE, FINGERPRINT_DIVIDE, 4);
    int x;
    int y;
    for (y = 0; y < FINGERPRINT_DIVIDE; y++)
    {
        for (x = 0; x < FINGERPRINT_DIVIDE; x++)
        {

            int blockX = floor((float)x * stepX);
            int blockY = floor((float)y * stepY);
            int bx;
            int by;
            uint64_t tr = 0;
            uint64_t tg = 0;
            uint64_t tb = 0;
            for (by = 0; by < stepYi; by++)
            {
                for (bx = 0; bx < stepXi; bx++)
                {
                    uint32_t p = surface->getPixel(blockX + bx, blockY + by);

                    uint8_t pr = ((p >> 16) & 0xff);
                    uint8_t pg = ((p >> 8) & 0xff);
                    uint8_t pb = ((p >> 0) & 0xff);

#if 0
                    tr += ((int)roundf((float)pr / 16.0f)) * 16;
                    tg += ((int)roundf((float)pg / 16.0f)) * 16;
                    tb += ((int)roundf((float)pb / 16.0f)) * 16;
#else
                    tr += pr & 0xff;
                    tg += pg & 0xff;
                    tb += pb & 0xff;
#endif
                }
            }
#if 0
            uint8_t r = ((int)((float)tr / (float)blockCount)) & 0xf0;
            uint8_t g = ((int)((float)tg / (float)blockCount)) & 0xf0;
            uint8_t b = ((int)((float)tb / (float)blockCount)) & 0xf0;
#else
            uint8_t r = ((int)(tr / blockCount)) & 0xc0;
            uint8_t g = ((int)(tg / blockCount)) & 0xc0;
            uint8_t b = ((int)(tb / blockCount)) & 0xc0;
#endif
            fingerprintSurface->drawPixel(x, y, 0xff000000 | (r << 16) | (g << 8) | (b << 0));
        }
    }

#if 0
    string fingerprintFile = "fingerprint_" + file + ".jpg";
    fingerprintSurface->saveJPEG(fingerprintFile);
#endif
    SHA1Context sha;
    uint8_t digest[20];
    int err;

    err = SHA1Reset(&sha);
    if (err)
    {
        return "";
    }

    err = SHA1Input(&sha, (uint8_t*)fingerprintSurface->getData(), fingerprintSurface->getDataLength());
    if (err)
    {
        return "";
    }

    err = SHA1Result(&sha, digest);
    if (err)
    {
        return "";
    }

    string digestStr = "";
    int i;
    for (i = 0; i < 20; i++)
    {
        char buf[3];
        sprintf(buf, "%02x", digest[i]);
        buf[2] = 0;
        digestStr += buf;
    }

    delete fingerprintSurface;
    delete surface;
    return digestStr;
}

