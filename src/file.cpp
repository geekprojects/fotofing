
#include <exiv2/exiv2.hpp>

#include "file.h"
#include "utils.h"
#include "sha1.h"

using namespace std;

#define FINGERPRINT_SIZE 12

union pixel_t
{
    uint32_t p;
    uint8_t rgb[4];
};

File::File(string path)
{
    m_path = path;
    m_thumbnail = NULL;
}

File::~File()
{
    if (m_thumbnail != NULL)
    {
        delete m_thumbnail;
    }
}

bool File::scan()
{
    Surface* image = Surface::loadJPEG(m_path);

    float imageWidth = (float)image->getWidth();
    float imageHeight = (float)image->getHeight();

    // Generate Thumbnail
    float ratio = imageHeight / imageWidth;

    int thumbWidth = 150;
    int thumbHeight = 150;
    if (ratio > 1)
    {
        printf(
            "File::generateThumbnail: Warning, ratio=%0.2f, please check",
            ratio);
        thumbWidth = (int)((float)thumbWidth * ratio);
    }
    else
    {
        thumbHeight = (int)((float)thumbWidth * ratio);
    }
#if 0
    printf("File::generateThumbnail: thumbWidth=%d\n", thumbWidth);
#endif

    m_thumbnail = new Surface(thumbWidth, thumbHeight, 4);

    float stepX = (float)imageWidth / (float)thumbWidth;
    float stepY = (float)imageHeight / (float)thumbHeight;

    float oy = 0;
    int y;
    int x;
    for (y = 0; y < thumbHeight; y++)
    {
        float ox = 0;
        for (x = 0; x < thumbWidth; x++)
        {
            uint32_t c = image->getPixel((int)ox, (int)oy);
            m_thumbnail->drawPixel(x, y, c);
            ox += stepX;
        }
        oy += stepY;
    }

    // Generate fingerprint
    stepX = (float)imageWidth / (float)FINGERPRINT_SIZE;
    stepY = (float)imageHeight / (float)FINGERPRINT_SIZE;
    int stepXi = (int)round(stepX);
    int stepYi = (int)round(stepY);
    int blockCount = (stepXi * stepYi);

    Surface* fingerprintSurface = new Surface(FINGERPRINT_SIZE, FINGERPRINT_SIZE, 4);
    for (y = 0; y < FINGERPRINT_SIZE; y++)
    {
        for (x = 0; x < FINGERPRINT_SIZE; x++)
        {

            int blockX = floor((float)x * stepX);
            int blockY = floor((float)y * stepY);
            int bx;
            int by;
            uint64_t totals[4];
memset(totals, 0, sizeof(uint64_t) * 4);
            for (by = 0; by < stepYi; by++)
            {
                for (bx = 0; bx < stepXi; bx++)
                {
                    pixel_t p;
                    p.p = image->getPixel(blockX + bx, blockY + by);

                    totals[0] += p.rgb[0];
                    totals[1] += p.rgb[1];
                    totals[2] += p.rgb[2];
                    totals[3] += p.rgb[3];

#if 0
                    tr += ((int)roundf((float)pr / 16.0f)) * 16;
                    tg += ((int)roundf((float)pg / 16.0f)) * 16;
                    tb += ((int)roundf((float)pb / 16.0f)) * 16;
#endif
                }
            }
#if 0
            uint8_t r = ((int)((float)tr / (float)blockCount)) & 0xf0;
            uint8_t g = ((int)((float)tg / (float)blockCount)) & 0xf0;
            uint8_t b = ((int)((float)tb / (float)blockCount)) & 0xf0;
#else
            uint8_t r = ((int)(totals[0] / blockCount)) & 0xc0;
            uint8_t g = ((int)(totals[1] / blockCount)) & 0xc0;
            uint8_t b = ((int)(totals[2] / blockCount)) & 0xc0;
#endif
            fingerprintSurface->drawPixel(x, y, 0xff000000 | (r << 16) | (g << 8) | (b << 0));
        }
    }

#if 0
    size_t pos = m_path.rfind('/');
    string file = m_path;
    if (pos != m_path.npos)
    {
        file = m_path.substr(pos + 1);
    }

    string fingerprintFile = "fingerprint_" + file + ".jpg";
    fingerprintSurface->saveJPEG(fingerprintFile);
#endif
    SHA1Context sha;
    uint8_t digest[20];
    memset(digest, 0, 20);
    int err;

    err = SHA1Reset(&sha);
    if (err)
    {
        return "";
    }

    err = SHA1Input(&sha, (uint8_t*)fingerprintSurface->getData(), fingerprintSurface->getDataLength());
    if (err)
    {
        return "";
    }

    err = SHA1Result(&sha, digest);
    if (err)
    {
        return "";
    }

    string digestStr = "";
    int i;
    for (i = 0; i < 20; i++)
    {
        char buf[3];
        sprintf(buf, "%02x", digest[i]);
        buf[2] = 0;
        digestStr += buf;
    }
    delete fingerprintSurface;

    m_fingerprint = string(digestStr);

    delete image;

    return true;
}

static string getTagValue(Exiv2::ExifData& exifData, string group, int tag, string def = "")
{
    string value = def;
    Exiv2::ExifData::const_iterator it;
    it = exifData.findKey(Exiv2::ExifKey(tag, group));
    if (it != exifData.end())
    {
        value = it->toString();
    }
    return value;
}

bool File::getTags(set<string>& tags, time_t* timestamp)
{
    // Derive tags from the EXIF data

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(m_path);
    image->readMetadata();

    Exiv2::ExifData &exifData = image->exifData();

    string make = getTagValue(exifData, "Image", 0x010f, "Unknown");
    string model = getTagValue(exifData, "Image", 0x0110, "Unknown");

    // Remove the make from the start of the model, if it's there
    // (It is with Canon)
    if (model.find(make) == 0)
    {
        model = model.substr(make.length());
        if (model.at(0) == ' ')
        {
            model = model.substr(1);
        }
    }

    string cameraTag = "Hardware/Camera/" + make + "/" + model;
    printf("Camera Tag: %s\n", cameraTag.c_str());
    tags.insert(cameraTag);

    // Lens details
    string lensMake = getTagValue(exifData, "Photo", 0xa433, "Unknown");
    string lensModel = getTagValue(exifData, "Photo", 0xa434, "Unknown");
    string lensTag = "Hardware/Lens/" + lensMake + "/" + lensModel;
    printf("Lens: %s\n", lensTag.c_str());
    tags.insert(lensTag);

    // Extract the timestamp
    string datetime = getTagValue(exifData, "Image", 0x0132);
    printf("datetime=%s\n", datetime.c_str());
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    strptime(datetime.c_str(), "%Y:%m:%d %H:%M%S", &tm);
    char datetimetag[128];
    sprintf(
        datetimetag,
        "Date/%d/%02d/%02d",
        tm.tm_year + 1900,
        tm.tm_mon + 1,
        tm.tm_mday);
    printf("Date Tag: %s\n", datetimetag);
    tags.insert(string(datetimetag));

    *timestamp = tm2time(&tm);
    printf("Timestamp : %lu\n", *timestamp);

    return true;
}

