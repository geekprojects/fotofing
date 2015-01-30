

#include "colourify.h"

using namespace std;
using namespace Geek::Gfx;

DECLARE_OPERATION(ColourifyOperation)

ColourifyOperation::ColourifyOperation()
{
}

ColourifyOperation::~ColourifyOperation()
{
}


OperationInstance* ColourifyOperation::createInstance()
{
    return new ColourifyInstance(this);
}


ColourifyInstance::ColourifyInstance(ColourifyOperation* op) :
    OperationInstance(op)
{
}

ColourifyInstance::~ColourifyInstance()
{
}


void ColourifyInstance::apply(Surface* surface, ProgressListener* prog)
{
    uint32_t* data = (uint32_t*)surface->getData();
    unsigned int y;
    for (y = 0; y < surface->getHeight(); y++)
    {
        unsigned int x;
        for (x = 0; x < surface->getWidth(); x++)
        {
            uint8_t r = *(data) >> 16;
            uint8_t g = *(data) >> 8;
            uint8_t b = *(data) >> 0;
            int avg = (r + g + b) / 3;
            *(data) = 0xff000000 | (avg << 16) | (avg << 8) | (avg);
            data++;
        }
    }
}

