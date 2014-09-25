#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cvaux.h>

#include <geek/core-xml.h>

#include <string>

using namespace std;
using namespace Geek::Core;

CvHaarClassifierCascade *cascade,*cascade_e,*cascade_nose,*cascade_mouth;
CvMemStorage            *storage;
const char *face_cascade = "haarcascade_frontalface_alt2.xml";
const char *eye_cascade = "haarcascade_mcs_eyepair_big.xml";
const char *nose_cascade = "haarcascade_mcs_nose.xml";
const char *mouth_cascade = "haarcascade_mcs_mouth.xml";

int detectMouth(IplImage *img, CvRect *r)
{
    CvSeq *mouth;
    cvSetImageROI(
        img,
        cvRect(
            r->x,            /* x = start from leftmost */
            r->y+(r->height *2/3), /* y = a few pixels from the top */
            r->width,        /* width = same width with the face */
            r->height/3));   /* height = 1/3 of face height */

    mouth = cvHaarDetectObjects(
        img,/* the source image, with the estimated location defined */
        cascade_mouth,      /* the eye classifier */
        storage,        /* memory buffer */
        1.15, 4, 0,     /* tune for your app */
        cvSize(25, 15));  /* minimum detection scale */
    if (mouth != NULL)
    {
        return mouth->total;
    }
    else
    {
        return 0;
    }
}

/*Nose detection*/
int detectNose(IplImage *img, CvRect *r)
{
    CvSeq* nose;

    //nose detection- set ROI
    cvSetImageROI(
        img,                    /* the source image */
        cvRect(
            r->x,            /* x = start from leftmost */
            r->y , /* y = a few pixels from the top */
            r->width,        /* width = same width with the face */
            r->height));  /* height = 1/3 of face height */
    nose = cvHaarDetectObjects(
        img, /* the source image, with the estimated location defined */
        cascade_nose,
        storage,
        1.15, 3, 0,
        cvSize(25, 15));  /* minimum detection scale */

    if (nose != NULL)
    {
        return nose->total;
    }
    else
    {
        return 0;
    }
}

int detectEyes(IplImage *img, CvRect *r)
{
    char *eyecascade;
    CvSeq *eyes;
    int eye_detect=0;

    //eye detection starts
    /* Set the Region of Interest: estimate the eyes' position */
    cvSetImageROI(
        img,                    /* the source image */
        cvRect(
            r->x,            /* x = start from leftmost */
            r->y + (r->height/5.5), /* y = a few pixels from the top */
            r->width,        /* width = same width with the face */
            r->height/3.0));    /* height = 1/3 of face height */

      /* detect the eyes */
      eyes = cvHaarDetectObjects(
          img,            /* the source image, with the estimated location defined */
          cascade_e,      /* the eye classifier */
          storage,        /* memory buffer */
          1.15, 3, 0,     /* tune for your app */
          cvSize(25, 15));  /* minimum detection scale */
    if (eyes != NULL)
    {
        return eyes->total;
    }
    else
    {
        return 0;
    }
}

void detectFacialFeatures(IplImage *img)
{
    char image[100],msg[100],temp_image[100];
    int w = (img)->width;
    int h = (img)->height;
    CvSeq* faces;

    CvMemStorage* storage = cvCreateMemStorage(0);
    cvClearMemStorage(storage);

    faces = cvHaarDetectObjects(
        img,
        cascade,
        storage,
        1.2,
        2,
        CV_HAAR_DO_CANNY_PRUNING,
        cvSize(20, 20));

    int completeFaces = 0;
    int i;
    for (i = 0; i < ( faces ? faces->total : 0 ) ; i++ )
    {
        CvRect* r = (CvRect*)cvGetSeqElem(faces, i);
        bool found;

        int noseCount = detectNose(img,r);
        cvResetImageROI(img);
        found = (noseCount == 1);

        if (found)
        {
            int mouthCount = detectMouth(img,r);
            cvResetImageROI(img);
            found = (mouthCount == 1);
        }

        if (found)
        {
            int eyeCount = detectEyes(img,r);
            cvResetImageROI(img);
            found = (eyeCount == 1);
        }

        if (found)
        {
#if 1
            cvRectangle(
                img,
                cvPoint( r->x, r->y ),
                cvPoint( r->x + r->width, r->y + r->height),
                CV_RGB( 255, 0, 0 ), 2, 8, 0 );

            printf("face_x=%d face_y=%d wd=%d ht=%d\n",r->x,r->y,r->width,r->height);
#endif
            completeFaces++;
        }

        cvResetImageROI(img);
    }

    /* reset region of interest */
    cvResetImageROI(img);

    if (completeFaces > 0)
    {
        printf("Feature/Faces/%d\n", completeFaces);
        cvSaveImage("output.jpg", img);
    }
}

CvHaarClassifierCascade* loadCascade(const char* name)
{
    string path = "/usr/share/opencv/haarcascades/" + string(name);
    return (CvHaarClassifierCascade*)cvLoad(path.c_str(), 0, 0, 0 );
}

int main( int argc, char** argv )
{
    CvCapture *capture;
    IplImage  *img;
    int       key;

XMLDocument config("config.xml");

xmlNodePtr features = config.evalPath("/opencvTaggers");


    storage = cvCreateMemStorage(0);
    cascade = loadCascade(face_cascade);
    cascade_e = loadCascade(eye_cascade);
    cascade_nose = loadCascade(nose_cascade);
    cascade_mouth = loadCascade(mouth_cascade);

    if( !(cascade || cascade_e ||cascade_nose||cascade_mouth) )
    {
        fprintf( stderr, "ERROR: Could not load classifier cascade\n" );
        return -1;
    }

    img = cvLoadImage(argv[1]);

    detectFacialFeatures(img);

    cvReleaseHaarClassifierCascade( &cascade );
    cvReleaseHaarClassifierCascade( &cascade_e );

    cvReleaseHaarClassifierCascade( &cascade_nose );
    cvReleaseHaarClassifierCascade( &cascade_mouth );
    cvReleaseMemStorage( &storage );

    cvReleaseImage(&img);

    return 0;
}
