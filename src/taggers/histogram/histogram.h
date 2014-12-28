#ifndef __FOTOFING_TAGGERS_HISTOGRAM_H_
#define __FOTOFING_TAGGERS_HISTOGRAM_H_

#include <fotofing/tagger.h>

class HistogramTagger : public Tagger
{
 private:

 public:
    HistogramTagger();
    virtual ~HistogramTagger();

    virtual bool tag(std::string path, Geek::Gfx::Surface* image, std::set<std::string>& tags);
};

#endif
