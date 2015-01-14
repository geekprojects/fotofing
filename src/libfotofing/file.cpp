
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
        thumbHeight = (int)((float)thumbHeight / ratio);
    }
    else
    {
        thumbWidth = (int)((float)thumbWidth * ratio);
    }

    m_thumbnail = m_image->scaleToFit(150, 150, false);

    Surface* fingerprintSurface = m_image->scaleToFit(FINGERPRINT_SIZE, FINGERPRINT_SIZE, true);

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

