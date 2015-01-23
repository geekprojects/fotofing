
#include <fotofing/workflow.h>

using namespace std;
using namespace Geek;
using namespace Geek::Gfx;

OperationInstance::OperationInstance(Operation* op)
{
    m_operation = op;
}

OperationInstance::~OperationInstance()
{
}


void OperationInstance::setAttribute(std::string name, int i)
{
}

void OperationInstance::setAttribute(std::string name, double d)
{
}

void OperationInstance::apply(
    Geek::Gfx::Surface* surface,
    ProgressListener* prog)
{
}

// Common utility methods
void OperationInstance::convolution(
    Surface* srcSurface,
    Surface* dstSurface,
    CentredMatrix* matrix)
{
    int matOffset = matrix->m_stride / 2;
    int border = matrix->m_stride / 2;
    uint8_t* dst = dstSurface->getData();

    int y1;
    for (y1 = 0; y1 < (int)srcSurface->getHeight(); y1++)
    {
        bool yborder = false;
        if (y1 < border || y1 >= (int)srcSurface->getHeight() - border)
        {
            yborder = true;
        }
        int x1;
        for (x1 = 0; x1 < (int)srcSurface->getWidth(); x1++)
        {
            double val[4] = {0.0, 0.0, 0.0, 0.0};

            double* mp = matrix->m_data;

            if (x1 > border && x1 < (int)srcSurface->getWidth() - border && !yborder)
            {
                uint8_t* src = srcSurface->getData() + srcSurface->getOffset(x1 - matOffset, y1 - matOffset);
                int y2;
                for (y2 = -matOffset; y2 <= matOffset; y2++)
                {
                    int x2;
                    for (x2 = -matOffset; x2 <= matOffset; x2++)
                    {
                        val[0] += *(mp) * (double)*(src++);
                        val[1] += *(mp) * (double)*(src++);
                        val[2] += *(mp) * (double)*(src++);
                        val[3] += *(mp) * (double)*(src++);
                        mp++;
                    }
                    src += (srcSurface->getWidth() - matrix->m_stride) * 4;
                }
                *(dst++) = CLAMP(val[0], 0, 255);
                *(dst++) = CLAMP(val[1], 0, 255);
                *(dst++) = CLAMP(val[2], 0, 255);
                *(dst++) = 255;
            }
            else
            {
                uint8_t* src = srcSurface->getData() + srcSurface->getOffset(x1, y1);
                *(dst++) = *(src++);
                *(dst++) = *(src++);
                *(dst++) = *(src++);
                *(dst++) = *(src++);
            }

        }
    }
}


