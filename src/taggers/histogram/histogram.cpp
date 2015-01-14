
#include <string.h>

#include "histogram.h"

#include <fotofing/index.h>

#define EXPOSURE_THRESHOLD 5.0f

DECLARE_TAGGER("Histogram", HistogramTagger);

using namespace std;

HistogramTagger::HistogramTagger()
{
}

HistogramTagger::~HistogramTagger()
{
}

#ifndef MAX
#define MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
#endif

bool HistogramTagger::tag(string path, Geek::Gfx::Surface* image, std::map<std::string, TagData*>& tags)
{
    uint64_t pixelCount = (image->getWidth() * image->getHeight());

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

            uint8_t r = (p >>  0) & 0xff;
            uint8_t g = (p >>  8) & 0xff;
            uint8_t b = (p >> 16) & 0xff;
            hist_r[r]++;
            hist_g[g]++;
            hist_b[b]++;
            max = MAX(max, hist_r[r]);
            max = MAX(max, hist_g[g]);
            max = MAX(max, hist_b[b]);
        }
    }

    float avg_r = 0.0f;
    float avg_g = 0.0f;
    float avg_b = 0.0f;

    float black = ((hist_r[0] + hist_g[0] + hist_b[0]) / (pixelCount * 3)) * 100.0f;
    float white = ((hist_r[255] + hist_g[255] + hist_b[255]) / (pixelCount * 3)) * 100.0f;
    printf("HistogramTagger::tag: Black=%0.2f%%\n", black);
    printf("HistogramTagger::tag: White=%0.2f%%\n", white);

    int i;
    for (i = 0; i < 256; i++)
    {
        avg_r += i * hist_r[i];
        avg_g += i * hist_g[i];
        avg_b += i * hist_b[i];
        hist_r[i] /= max;
        hist_g[i] /= max;
        hist_b[i] /= max;
    }

    avg_r /= pixelCount;
    avg_g /= pixelCount;
    avg_b /= pixelCount;
    float avg = (avg_r + avg_g + avg_b) / 3.0f;

#if 0
    printf("HistogramTagger::tag: Average red  =%0.2f\n", avg_r);
    printf("HistogramTagger::tag: Average green=%0.2f\n", avg_g);
    printf("HistogramTagger::tag: Average blue =%0.2f\n", avg_b);
    printf("HistogramTagger::tag: Average      =%0.2f\n", avg);
#endif

    //hist_r[(int)avg_r] = 1.0f;
    //hist_g[(int)avg_g] = 1.0f;
    //hist_b[(int)avg_b] = 1.0f;

    tags.insert(
        make_pair(
            "Fotofing/Taggers/Histogram/Red",
            new TagData(hist_r, sizeof(float) * 256)));
    tags.insert(
        make_pair(
            "Fotofing/Taggers/Histogram/Green",
            new TagData(hist_g, sizeof(float) * 256)));
    tags.insert(
        make_pair(
            "Fotofing/Taggers/Histogram/Blue",
            new TagData(hist_b, sizeof(float) * 256)));

    if (white >= EXPOSURE_THRESHOLD)
    {
        tags.insert( make_pair("Photo/Exposure/Over Exposed", new TagData()));
    }
    if (black >= EXPOSURE_THRESHOLD)
    {
        tags.insert( make_pair("Photo/Exposure/Under Exposed", new TagData()));
    }

    if (white < 1.0f && black < 1.0f && fabs(avg - 128.0f) < 64.0f)
    {
        tags.insert( make_pair("Photo/Exposure/Well Exposed", new TagData()));
    }

    return true;
}

