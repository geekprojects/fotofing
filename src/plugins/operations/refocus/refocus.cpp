
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

extern "C" {
#include <clapack.h>
}

#include <geek/gfx-surface.h>
#include <geek/core-matrix.h>

using namespace Geek;
using namespace Geek::Gfx;

int dgesv (
    const int N,
    const int NRHS,
    double *A,
    const int lda,
    double *B,
    const int ldb)
{
    int result = 0;
    int* ipiv = new int[N];
    result = clapack_dgesv (CblasColMajor, N, NRHS, A, lda, ipiv, B, ldb);
    delete[] ipiv;
    return result;
}

void make_gaussian_convolution(CentredMatrix* matrix, const double gradius)
{
    int x, y;

    if (SQR (gradius) <= 1 / FLT_MAX)
    {
        for (y = -matrix->m_radius; y <= matrix->m_radius; y++)
        {
            for (x = -matrix->m_radius; x <= matrix->m_radius; x++)
            {
                matrix->set(x, y, 0);
            }
        }
        matrix->set(0, 0, 1);
    }
    else
    {
        const double alpha = log (2.0) / SQR (gradius);
        for (y = -matrix->m_radius; y <= matrix->m_radius; y++)
        {
            for (x = -matrix->m_radius; x <= matrix->m_radius; x++)
            {
                matrix->set(x, y, exp (-alpha * (SQR (x) + SQR (y))));
            }
        }
    }
}

/* Return the integral of sqrt(radius^2 - z^2) for z = 0 to x */
double circle_integral (const double x, const double radius)
{
    if (radius == 0)
    {                           /* Perhaps some epsilon must be added here */
        return (0);
    }
    else
    {
        const double sin = x / radius;
        const double sq_diff = SQR (radius) - SQR (x);
        /* From a mathematical point of view the following is redundant.
           Numerically they are not equivalent!
         */
        if ((sq_diff < 0.0) || (sin < -1.0) || (sin > 1.0))
        {
            if (sin < 0)
            {
                return (-0.25 * SQR (radius) * M_PI);
            }
            else
            {
                return (0.25 * SQR (radius) * M_PI);
            }
        }
        else
        {
            return (0.5 * x * sqrt (sq_diff) + 0.5 * SQR (radius) * asin (sin));
        }
    }
}

double circle_intensity (const int x, const int y, const double radius)
{
    if (radius == 0)
    {
        return (((x == 0) && (y == 0)) ? 1 : 0);
    }
    else
    {
        double xlo = ABS(x) - 0.5;
        double xhi = ABS(x) + 0.5;
        double ylo = ABS(y) - 0.5;
        double yhi = ABS(y) + 0.5;
        double symmetry_factor = 1, xc1, xc2;

        if (xlo < 0)
        {
            xlo = 0;
            symmetry_factor *= 2;
        }

        if (ylo < 0)
        {
            ylo = 0;
            symmetry_factor *= 2;
        }

        if (SQR (xlo) + SQR (yhi) > SQR (radius))
        {
            xc1 = xlo;
        }
        else if (SQR (xhi) + SQR (yhi) > SQR (radius))
        {
            xc1 = sqrt (SQR (radius) - SQR (yhi));
        }
        else
        {
            xc1 = xhi;
        }

        if (SQR (xlo) + SQR (ylo) > SQR (radius))
        {
            xc2 = xlo;
        }
        else if (SQR (xhi) + SQR (ylo) > SQR (radius))
        {
            xc2 = sqrt (SQR (radius) - SQR (ylo));
        }
        else
        {
            xc2 = xhi;
        }

        double result;
        result  = (yhi - ylo) * (xc1 - xlo);
        result += circle_integral(xc2, radius);
        result -= circle_integral(xc1, radius);
        result -= (xc2 - xc1) * ylo;
        result *= symmetry_factor / (M_PI * SQR (radius));
        return result;
    }
}

void make_circle_convolution(CentredMatrix* cmatrix, int matrixWidth)
{
    cmatrix->fill(circle_intensity, matrixWidth);
}

double correlation (const int x, const int y, const double gamma, const double musq)
{
  return (musq + pow (gamma, sqrt (SQR (x) + SQR (y))));
}

CentredMatrix* compute_g (const CentredMatrix* const convolution, const int m, const double gamma,
           const double noise_factor, const double musq, const bool symmetric)
{
    CentredMatrix h_conv_ruv(3 * m);
    CentredMatrix a(2 * m);
    CentredMatrix corr(4 * m);
    CentredMatrix *result;
    Matrix* b;
    Matrix* s;

    corr.fill(correlation, gamma, musq);
    h_conv_ruv.convolve_mat(convolution, &corr);
    a.convolve_star_mat(convolution, &h_conv_ruv);
    if (symmetric)
    {
        s = a.makeSCentredMatrix(m, noise_factor);
        b = h_conv_ruv.copyCVec(m);
    }
    else
    {
        s = a.makeSMatrix(m, noise_factor);
        b = h_conv_ruv.copyVec(m);
    }

    dgesv (s->m_rows, 1, s->m_data, s->m_rows, b->m_data, b->m_rows);

    if (symmetric)
    {
        result = b->copyCVec2Mat(m);
    }
    else
    {
        result = b->copyVec2Mat(m);
    }

    delete s;
    delete b;

    return result;
}

CentredMatrix* compute_g_matrix(
    const CentredMatrix* const convolution,
    const int m,
    const double gamma,
    const double noise_factor,
    const double musq,
    const bool symmetric)
{
    CentredMatrix *g = compute_g (convolution, m, gamma, noise_factor, musq, symmetric);
    int r, c;
    double sum = 0.0;

    /* Determine sum of array */
    for (r = -g->m_radius; r <= g->m_radius; r++)
    {
        for (c = -g->m_radius; c <= g->m_radius; c++)
        {
            sum += g->get(r, c);
        }
    }
    for (r = -g->m_radius; r <= g->m_radius; r++)
    {
        for (c = -g->m_radius; c <= g->m_radius; c++)
        {
            g->set(r, c, g->get(r, c) / sum);
        }
    }
    return (g);
}


int main(int argc, char** argv)
{
    Surface* srcSurface = Surface::loadJPEG("test.jpg");
    Surface* dstSurface = new Surface(srcSurface->getWidth(), srcSurface->getHeight(), 4);

    int matrix_width = 5;
    double radius = 1;
    double alpha = 0.0;
    double gamma = 0.5;
    double noiseFactor = 0.01;

    // Circular
    CentredMatrix cmatrix(matrix_width);
    CentredMatrix gmatrix(matrix_width);

    make_circle_convolution(&cmatrix, radius);
    make_gaussian_convolution(&gmatrix, alpha);

    CentredMatrix convolution(matrix_width);
    convolution.convolve_star_mat(&gmatrix, &cmatrix);

    convolution.dump();
    printf("\n");

CentredMatrix* matrix;
      matrix = compute_g_matrix (&convolution, matrix_width,
                                 gamma,
                                 noiseFactor, 0.0, true);

    matrix->dump();
    printf("\n");

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
                *(dst++) = CLAMP(val[2], 0, 255);
                *(dst++) = CLAMP(val[1], 0, 255);
                *(dst++) = CLAMP(val[0], 0, 255);
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

    delete matrix;

    dstSurface->saveJPEG("output.jpg");

    delete srcSurface;
    delete dstSurface;
}


