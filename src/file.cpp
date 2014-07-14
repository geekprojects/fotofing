
#include <exiv2/exiv2.hpp>

#include "file.h"

using namespace std;

File::File(string path)
{
    m_path = path;
}

File::~File()
{
}

Surface* File::generateThumbnail()
{
    Surface* image = Surface::loadJPEG(m_path);

    float imageWidth = (float)image->getWidth();
    float imageHeight = (float)image->getHeight();

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
    printf("File::generateThumbnail: thumbWidth=%d\n", thumbWidth);

    Surface* thumbnail = new Surface(thumbWidth, thumbHeight, 4);

    float stepX = (float)imageWidth / (float)thumbWidth;
    float stepY = (float)imageHeight / (float)thumbHeight;

    float oy = 0;
    int y;
    for (y = 0; y < thumbHeight; y++)
    {
        float ox = 0;
        int x;
        for (x = 0; x < thumbWidth; x++)
        {
            uint32_t c = image->getPixel((int)ox, (int)oy);
            thumbnail->drawPixel(x, y, c);
            ox += stepX;
        }
        oy += stepY;
    }
    delete image;

    size_t pos = m_path.rfind('/');
    string file = m_path;
    if (pos != m_path.npos)
    {
        file = m_path.substr(pos + 1);
    }
    string thumbFile = "thumbs/thumb_" + file + ".jpg";
    thumbnail->saveJPEG(thumbFile);

    return thumbnail;
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

    string lensMake = getTagValue(exifData, "Photo", 0xa433, "Unknown");
    string lensModel = getTagValue(exifData, "Photo", 0xa434, "Unknown");
    string lensTag = "Hardware/Lens/" + lensMake + "/" + lensModel;
    printf("Lens: %s\n", lensTag.c_str());
    tags.insert(lensTag);

    string datetime = getTagValue(exifData, "Image", 0x0132);
    printf("datetime=%s\n", datetime.c_str());
    struct tm tm;
    strptime(datetime.c_str(), "%Y:%m:%d %H:%M%S", &tm);
    //*timestamp = mktime(&tm);
    tm.tm_year += 1900;
    char datetimetag[128];
    sprintf(datetimetag, "Date/%d/%02d/%02d", tm.tm_year, tm.tm_mon, tm.tm_mday);
    printf("Date Tag: %s\n", datetimetag);
    tags.insert(string(datetimetag));

    return true;
}

