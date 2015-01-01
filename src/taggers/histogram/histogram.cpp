
#include <string.h>

#include "histogram.h"

DECLARE_TAGGER("Histogram", HistogramTagger);

using namespace std;

HistogramTagger::HistogramTagger()
{
}

HistogramTagger::~HistogramTagger()
{
}

//#define MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))

bool HistogramTagger::tag(string path, Geek::Gfx::Surface* image, std::map<std::string, TagData*>& tags)
{
    float hist_r[256];
    float hist_g[256];
    float hist_b[256];
    float max = 0;

    memset(hist_r, 0, sizeof(float) * 256);
    memset(hist_g, 0, sizeof(float) * 256);
    memset(hist_b, 0, sizeof(float) * 256);

    uint32_t* data = (uint32_t*)image->getData();

    int y;
    for (y = 0; y < image->getHeight(); y++)
    {
        int x;
        for (x = 0; x < image->getWidth(); x++)
        {
            uint32_t p = *(data++);

            uint8_t r = (p >> 16) & 0xff;
            uint8_t g = (p >>  8) & 0xff;
            uint8_t b = (p >>  0) & 0xff;
            hist_r[r]++;
            hist_g[g]++;
            hist_b[b]++;
            max = MAX(max, hist_r[r]);
            max = MAX(max, hist_g[g]);
            max = MAX(max, hist_b[b]);
        }
    }

    int i;
    for (i = 0; i < 256; i++)
    {
        hist_r[i] /= max;
        hist_g[i] /= max;
        hist_b[i] /= max;
        printf("%-3d: %f %f %f\n", i, hist_r[i], hist_g[i], hist_b[i]);
    }

    return true;
}

