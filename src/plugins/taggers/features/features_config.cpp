
#include "features_config.h"

using namespace std;
using namespace Geek::Core;

Config::Config(string path) :
    XMLDocument(path)
{
}

Config::~Config()
{
}

bool Config::parse()
{
    vector<xmlNodePtr> features = evalPath("/features/feature");

    vector<xmlNodePtr>::iterator it;
    for (it = features.begin(); it != features.end(); it++)
    {
        xmlNodePtr node = *it;
        Feature feature = parseFeature(node);
        m_features.push_back(feature);
    }

    return true;
}

Feature Config::parseFeature(xmlNodePtr node)
{
    xmlChar* name = xmlGetProp(node, (const xmlChar*)"name");
    xmlChar* tag = xmlGetProp(node, (const xmlChar*)"tag");
    printf("Config::parseFeature: feature: %s\n", name);
    Feature feature;
    if (name != NULL)
    {
        feature.name = string((const char*)name);
    }
    if (tag != NULL)
    {
        feature.tag = string((const char*)tag);
    }

    xmlNodePtr cascadeNode = evalPathFirst("cascade", node);
    if (cascadeNode != NULL)
    {

        xmlNodePtr defNode = evalPathFirst("definition", cascadeNode);
        xmlChar* cascadeBase = xmlGetProp(defNode, (const xmlChar*)"base");
        xmlChar* cascadePath = defNode->children->content;

        // Trim the cascadePath
        while (*cascadePath == ' ' || *cascadePath == '\n')
        {
            cascadePath++;
        }

        int i = strlen((const char*)cascadePath) - 1;
        while (cascadePath[i] == ' ' || cascadePath[i] == '\n')
        {
            cascadePath[i] = '\0';
            i--;
        }

        string path = "";
        if (!strcmp("opencv", (char*)cascadeBase))
        {
            path = "/usr/share/opencv/haarcascades/";
        }

        path += string((char*)cascadePath);

        printf("Config::parseFeature: Loading cascade: %s\n", path.c_str());
        feature.cascade = (CvHaarClassifierCascade*)cvLoad(
            path.c_str(),
            0, 0, 0 );
        if (feature.cascade == NULL)
        {
            printf("Config::parseFeature: WARNING: Unable to load cascade: %s\n", path.c_str());
        }

        string scaleFactorStr = readTextNode("scaleFactor", cascadeNode);
        string minNeighboursStr = readTextNode("minNeighbours", cascadeNode);
        string flagsStr = readTextNode("flags", cascadeNode);
        string minWidthStr = readTextNode("minWidth", cascadeNode);
        string minHeightStr = readTextNode("minHeight", cascadeNode);

        feature.scaleFactor = 1.0f;
        feature.minNeighbours = 0;
        feature.minWidth = 0;
        feature.minHeight = 0;
        feature.flags = 0;

        if (scaleFactorStr.length() > 0)
        {
            feature.scaleFactor = atof(scaleFactorStr.c_str());
        }

        if (minNeighboursStr.length() > 0)
        {
            feature.minNeighbours = atoi(minNeighboursStr.c_str());
        }

        if (minWidthStr.length() > 0)
        {
            feature.minWidth = atoi(minWidthStr.c_str());
        }

        if (minHeightStr.length() > 0)
        {
            feature.minHeight = atoi(minHeightStr.c_str());
        }

        if (strstr(flagsStr.c_str(), "DO_CANNY_PRUNING") != NULL)
        {
            feature.flags |= CV_HAAR_DO_CANNY_PRUNING;
        }
    }

    vector<xmlNodePtr> subfeatures = evalPath("subfeatures/feature", node);
    vector<xmlNodePtr>::iterator subit;
    for (subit = subfeatures.begin(); subit != subfeatures.end(); subit++)
    {
        xmlNodePtr subnode = *subit;
        Feature subfeature = parseFeature(subnode);
        feature.subfeatures.push_back(subfeature);
    }
    return feature;
}

