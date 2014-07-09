
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

    printf("File::generateThumbnail: imageWidth=%0.2f\n", imageWidth);
    printf("File::generateThumbnail: imageHeight=%0.2f\n", imageHeight);
    printf("File::generateThumbnail: ratio=%0.2f\n", ratio);

    int thumbWidth = 150;
    int thumbHeight = 150;
    if (ratio > 1)
    {
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

    size_t pos = m_path.rfind('/');
    string file = m_path;
    if (pos != m_path.npos)
    {
        file = m_path.substr(pos + 1);
    }
    string thumbFile = "thumbs/thumb_" + file + ".jpg";
    thumbnail->saveJPEG(thumbFile);

    delete image;

    return thumbnail;
}

