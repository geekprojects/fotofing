#ifndef __FOTOFING_FEATURES_CONFIG_H_
#define __FOTOFING_FEATURES_CONFIG_H_

#include <geek/core-xml.h>

#include <opencv/cv.h>

struct Feature
{
    std::string name;
    std::string tag;

    CvHaarClassifierCascade* cascade;
    double scaleFactor;
    int minNeighbours;
    int flags;
    int minWidth;
    int minHeight;

    std::vector<Feature> subfeatures;
};

class Config : public Geek::Core::XMLDocument
{
 private:
    std::vector<Feature> m_features;

    Feature parseFeature(xmlNodePtr node);


 public:
    Config(std::string path);
    ~Config();

    bool parse();

    std::vector<Feature>& getFeatures() { return m_features; }
};

#endif
