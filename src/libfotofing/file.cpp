
#include <exiv2/exiv2.hpp>

#include <fotofing/file.h>
#include <fotofing/utils.h>
#include <fotofing/tagger.h>
#include "sha1.h"

using namespace std;
using namespace Geek::Gfx;

#define FINGERPRINT_SIZE 12

typedef int v4si __attribute__ ((vector_size (16)));

File::File(int64_t sourceId, string path)
{
    m_sourceId = sourceId;
    m_path = path;
    m_thumbnail = NULL;

    m_image = NULL;
}

File::~File()
{
    if (m_thumbnail != NULL)
    {
        delete m_thumbnail;
    }

    if (m_image != NULL)
    {
        delete m_image;
    }
}

bool File::scan()
{
    m_image = Surface::loadJPEG(m_path);

    float imageWidth = (float)m_image->getWidth();
    float imageHeight = (float)m_image->getHeight();
    float ratio = imageWidth / imageHeight;

    int thumbWidth = 150;
    int thumbHeight = 150;
    if (ratio > 1.0f)
    {
        printf(
            "File::generateThumbnail: Warning, ratio=%0.2f, please check\n",
            ratio);
        thumbHeight = (int)((float)thumbHeight / ratio);
    }
    else
    {
        thumbWidth = (int)((float)thumbWidth * ratio);
    }

    m_thumbnail = generateThumbnail(m_image, thumbWidth, thumbHeight, false);
    Surface* fingerprintSurface = generateThumbnail(m_image, FINGERPRINT_SIZE, FINGERPRINT_SIZE, true);

#if 0
    size_t pos = m_path.rfind('/');
    string file = m_path;
    if (pos != m_path.npos)
    {
        file = m_path.substr(pos + 1);
    }

    string fingerprintFile = "fingerprint_" + file + ".jpg";
    fingerprintSurface->saveJPEG(fingerprintFile);

    string thumbnailFile = "thumbnail_" + file + ".jpg";
    m_thumbnail->saveJPEG(thumbnailFile);
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

    return true;
}

Surface* File::generateThumbnail(Surface* image, int thumbWidth, int thumbHeight, bool fingerprint)
{
    int imageWidthi = image->getWidth();
    float imageWidth = (float)image->getWidth();
    float imageHeight = (float)image->getHeight();

    // Generate fingerprint
    float stepX = (float)imageWidth / (float)thumbWidth;
    float stepY = (float)imageHeight / (float)thumbHeight;
    int stepXi = (int)round(stepX);
    int stepYi = (int)round(stepY);
    int blockCount = (stepXi * stepYi);

    Surface* fingerprintSurface = new Surface(thumbWidth, thumbHeight, 4);

    uint32_t* data = (uint32_t*)fingerprintSurface->getData();
    uint8_t* srcdata = (uint8_t*)image->getData();

int blockDelta = (imageWidthi - stepXi) * 4;

    int y;
    for (y = 0; y < thumbHeight; y++)
    {
        int blockY = floor((float)y * stepY);
        int x;
        for (x = 0; x < thumbWidth; x++)
        {

            int blockX = floor((float)x * stepX);
            int bx;
            int by;
            v4si totals = {0, 0, 0, 0};

            uint8_t* imgrow = srcdata;
            imgrow += ((imageWidthi * blockY) + (blockX)) * 4;

            for (by = 0; by < stepYi; by++)
            {
                for (bx = 0; bx < stepXi; bx++)
                {
                    v4si pv = {
                        imgrow[3],
                        imgrow[2],
                        imgrow[1],
                        imgrow[0]};
                    totals += pv;
                    imgrow += 4;
                }
                imgrow += blockDelta;
            }
            v4si avg = totals / blockCount;
            if (fingerprint)
            {
                avg &= 0xc0;
            }

            *(data++) = 0xff000000 | (avg[1] << 16) | (avg[2] << 8) | (avg[3] << 0);
        }
    }

    return fingerprintSurface;
}

bool File::getTags(map<string, TagData*>& tags)
{
    vector<TaggerInfo*> taggers = Tagger::findTaggers();
    vector<TaggerInfo*>::iterator it;
    for (it = taggers.begin(); it != taggers.end(); it++)
    {
        TaggerInfo* info = *it;
        if (info->tagger == NULL)
        {
            info->tagger = info->newTagger();
        }

        bool res;
        res = info->tagger->tag(m_path, m_image, tags);
        if (res)
        {
            tags.insert(make_pair(string("Fotofing/Taggers/") + info->name, (TagData*)NULL));
        }
    }

    return true;
}

