
#include "histogram.h"

DECLARE_TAGGER("Histogram", HistogramTagger);

using namespace std;

HistogramTagger::HistogramTagger()
{
    printf("HistogramTagger::HistogramTagger: Here!\n");
}

HistogramTagger::~HistogramTagger()
{
    printf("HistogramTagger::~HistogramTagger: Here!\n");
}

void HistogramTagger::tag(string path, Geek::Gfx::Surface* image)
{
    printf("HistogramTagger::tag: Here!\n");
}


