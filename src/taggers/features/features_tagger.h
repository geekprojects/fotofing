#ifndef __TAGGERS_FEATURES_H_
#define __TAGGERS_FEATURES_H_

#include "features_config.h"

#include <string>

class FeatureTagger
{
 private:
    Config* m_config;

    CvMemStorage* m_storage;

    int matches(IplImage* img, Feature feature);

 public:
    FeatureTagger();
    ~FeatureTagger();

    bool init();

    std::vector<std::string> tag(std::string path);
};

#endif
