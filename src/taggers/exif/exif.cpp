
#include <string.h>

#include <exiv2/exiv2.hpp>

#include <fotofing/file.h>
#include <fotofing/utils.h>
#include <fotofing/index.h>

#include "exif.h"
#include "exiftags.h"

DECLARE_TAGGER("Exif", ExifTagger);

using namespace std;

ExifTagger::ExifTagger()
{
    printf("ExifTagger::ExifTagger: Here!\n");
}

ExifTagger::~ExifTagger()
{
    printf("ExifTagger::~ExifTagger: Here!\n");
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

bool ExifTagger::tag(string path, Geek::Gfx::Surface* image, std::map<std::string, TagData*>& tags)
{
    printf("ExifTagger::tag: Here!\n");

    // Derive tags from the EXIF data
    Exiv2::Image::AutoPtr exifImage = Exiv2::ImageFactory::open(path);
    exifImage->readMetadata();

    Exiv2::ExifData &exifData = exifImage->exifData();
    if (exifData.empty())
    {
        return false;
    }

    string make = getTagValue(exifData, EXIF_Image_Make, "Unknown");
    string model = getTagValue(exifData, EXIF_Image_Model, "Unknown");

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
    tags.insert(make_pair(cameraTag, (TagData*)NULL));

    // Lens details
    string lensMake = getTagValue(exifData, EXIF_Photo_LensMake, "Unknown");
    string lensModel = getTagValue(exifData, EXIF_Photo_LensModel, "Unknown");
    string lensTag = "Hardware/Lens/" + lensMake + "/" + lensModel;
    tags.insert(make_pair(lensTag, (TagData*)NULL));

    // Extract the timestamp
    string datetime = getTagValue(exifData, EXIF_Image_DateTimeOriginal);
    if (datetime == "")
    {
        datetime = getTagValue(exifData, EXIF_Image_DateTime);
    }

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
    tags.insert(make_pair("Date", new TagData(tm2time(&tm))));
    tags.insert(make_pair(string(datetimetag), (TagData*)NULL));

    string serialNumber = "";
    if (make == "Canon")
    {
        serialNumber = getTagValue(exifData, EXIF_Canon_SerialNumber);
        if (serialNumber == "")
        {
            // In some Canon cameras (newer?) such as the 70D, we have to use
            // the Internal Serial Number
            serialNumber = getTagValue(
                exifData,
                EXIF_Canon_InternalSerialNumber);
        }
    }
    else if (make == "Panasonic")
    {
        serialNumber = getTagValue(
            exifData,
            EXIF_Panasonic_InternalSerialNumber);
    }

    if (serialNumber == "")
    {
        // If all else fails, try the generic "BodySerialNumber" tag
        // (Often empty if the manufacturer specific tag has a value)
        serialNumber = getTagValue(
            exifData,
            EXIF_Photo_BodySerialNumber,
            "Unknown");
    }
    tags.insert(make_pair("Hardware/Camera/Serial/" + serialNumber, (TagData*)NULL));

    return true;
}

