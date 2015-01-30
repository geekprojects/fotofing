#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <string>

#include "features_tagger.h"
#include "features_config.h"

using namespace std;
using namespace Geek::Core;

FeatureTagger::FeatureTagger()
{
}

FeatureTagger::~FeatureTagger()
{
}

bool FeatureTagger::init()
{
    m_config = new Config("config.xml");
    m_config->parse();
}

int FeatureTagger::matches(IplImage* img, Feature feature)
{
    printf("FeatureTagger::matches: %s: matching...\n", feature.name.c_str());

#if 1
    printf("FeatureTagger::matches: %s: scaleFactor=%0.2f, minNeighbours=%d, flags=%d, minWidth=%d, minHeight=%d\n", feature.name.c_str(), feature.scaleFactor, feature.minNeighbours, feature.flags, feature.minWidth, feature.minHeight);
#endif

    CvSeq* objs;
    objs = cvHaarDetectObjects(
        img,
        feature.cascade,
        m_storage,
        feature.scaleFactor,
        feature.minNeighbours,
        feature.flags,
        cvSize(feature.minWidth, feature.minHeight));

    printf("FeatureTagger::matches: %s: matches=%d\n", feature.name.c_str(), (objs != NULL) ? objs->total : 0);

    if (objs != NULL && objs->total > 0)
    {
        int matchCount = objs->total;
        int i;

        if (!feature.subfeatures.empty())
        {
            matchCount = 0;
            for (i = 0; i < objs->total; i++)
            {
                CvRect* r = (CvRect*)cvGetSeqElem(objs, i);

                cvSetImageROI(
                    img,
                    cvRect(r->x, r->y, r->width, r->height));

                vector<Feature>::iterator it;
                bool match = true;
                for (it = feature.subfeatures.begin();
                    it != feature.subfeatures.end();
                    it++)
                {
                    printf("FeatureTagger::matches: Checking subfeature: %s\n", it->name.c_str());
                    int res = matches(img, *it);
                    match = (res == 1);

                    if (!match)
                    {
                        break;
                    }
                }
                if (match)
                {
                    printf("Matches subfeatures!\n");
                    matchCount++;
                }
                cvResetImageROI(img);
            }
        }

#if 1
//if (matchCount > 0)
//{
        for (i = 0; i < objs->total; i++)
        {
            CvRect* r = (CvRect*)cvGetSeqElem(objs, i);
            cvRectangle(
                img,
                cvPoint(r->x, r->y),
                cvPoint(r->x + r->width, r->y + r->height),
                CV_RGB(255, 0, 0),
                2, 8, 0);
        }
//}
#endif

        return matchCount;
    }
    else
    {
        return 0;
    }
}

vector<string> FeatureTagger::tag(string path)
{
    vector<string> results;
    IplImage  *img;

    img = cvLoadImage(path.c_str(), CV_LOAD_IMAGE_GRAYSCALE);

#if 0
double scale = 1.0f;
cv::Mat frame = img;;
//cv::Mat frameCopy;
//frame.copyTo(frameCopy);

    const static cv::Scalar colors[] =  { CV_RGB(0,0,255),
        CV_RGB(0,128,255),
        CV_RGB(0,255,255),
        CV_RGB(0,255,0),
        CV_RGB(255,128,0),
        CV_RGB(255,255,0),
        CV_RGB(255,0,0),
        CV_RGB(255,0,255)} ;
    cv::Mat gray, smallImg( cvRound (frame.rows/scale), cvRound(frame.cols/scale), CV_8UC1 );

    cvtColor(frame, gray, CV_BGR2GRAY );
    cv::resize( gray, smallImg, smallImg.size(), 0, 0, cv::INTER_LINEAR );
    cv::equalizeHist( smallImg, smallImg );

cvSaveImage("frame.jpg", &frame);
cvSaveImage("smallImg.jpg", &smallImg);
#endif

    m_storage = cvCreateMemStorage(0);
    cvClearMemStorage(m_storage);

    vector<Feature> features = m_config->getFeatures();
    vector<Feature>::iterator it;
    for (it = features.begin(); it != features.end(); it++)
    {
        Feature feature = *it;
        int count;
printf("FeatureTagger::tag: Feature: %s\n", feature.name.c_str());
        count = matches(img, feature);
        if (count > 0)
        {
            printf("FeatureTagger::tag: %s/%d\n", feature.tag.c_str(), count);
        }
    }
cvSaveImage("output.jpg", img);

    cvReleaseImage(&img);
    return results;
}

int main( int argc, char** argv )
{

    FeatureTagger tagger;
    tagger.init();
    printf("FeatureTagger: Tagging...\n");
    tagger.tag(argv[1]);

    return 0;
}

