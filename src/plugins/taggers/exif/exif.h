#ifndef __FOTOFING_TAGGERS_EXIF_H_
#define __FOTOFING_TAGGERS_EXIF_H_

#include <fotofing/tagger.h>

class ExifTagger : public Tagger
{
 private:

 public:
    ExifTagger();
    virtual ~ExifTagger();

    virtual bool tag(std::string path, Geek::Gfx::Surface* image, std::map<std::string, TagData*>& tags);
};

#endif
