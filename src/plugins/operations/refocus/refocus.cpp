/*
 * Refocus operation for FotoFing
 *
 * Based on the GIMP refocus plugin by
 * Ernst Lippe <ernstl@users.sourceforge.net>
 */
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

#include "refocus.h"

using namespace std;
using namespace Geek;
using namespace Geek::Gfx;

DECLARE_OPERATION(RefocusOperation)

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

double correlation(
    const int x,
    const int y,
    const double gamma,
    const double musq)
{
    return (musq + pow (gamma, sqrt (SQR (x) + SQR (y))));
}

CentredMatrix* compute_g (
    const CentredMatrix* const convolution,
    const int m,
    const double gamma,
    const double noise_factor,
    const double musq,
    const bool symmetric)
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

RefocusOperation::RefocusOperation()
{
}

RefocusOperation::~RefocusOperation()
{
}

vector<OperationAttribute> RefocusOperation::getAttributes()
{
    vector<OperationAttribute> attrs;

    OperationAttribute attr;
    attr.name = REFOCUS_ATTR_MATRIXSIZE;
    attr.label = "Matrix Size";
    attr.description = "Size of the Matrix";
    attr.type = OPERATION_ATTR_TYPE_INT;
    attr.min.i = 1;
    attr.max.i = 25;
    attr.def.i = 5;
    attrs.push_back(attr);

    attr.name = REFOCUS_ATTR_RADIUS;
    attr.label = "Radius";
    attr.description = "Radius for the Circle Convolution";
    attr.type = OPERATION_ATTR_TYPE_DOUBLE;
    attr.min.d = 0.0;
    attr.max.d = 25.0;
    attr.def.d = 1.0;
    attrs.push_back(attr);

    attr.name = REFOCUS_ATTR_GAUSS;
    attr.label = "Gauss";
    attr.description = "Gauss Convolution Value";
    attr.type = OPERATION_ATTR_TYPE_DOUBLE;
    attr.min.d = 0.0;
    attr.max.d = 25.0;
    attr.def.d = 0.0;
    attrs.push_back(attr);

    attr.name = REFOCUS_ATTR_CORRELATION;
    attr.label = "Correlation";
    attr.description = "Correlation";
    attr.type = OPERATION_ATTR_TYPE_DOUBLE;
    attr.min.d = 0.0;
    attr.max.d = 25.0;
    attr.def.d = 0.5;
    attrs.push_back(attr);

    attr.name = REFOCUS_ATTR_NOISE;
    attr.label = "Noise";
    attr.description = "Signal to Noise Ratio";
    attr.type = OPERATION_ATTR_TYPE_DOUBLE;
    attr.min.d = 0.0;
    attr.max.d = 25.0;
    attr.def.d = 0.01;
    attrs.push_back(attr);

    return attrs;
}

OperationInstance* RefocusOperation::createInstance()
{
    return new RefocusInstance(this);
}

RefocusInstance::RefocusInstance(RefocusOperation* op)
    : OperationInstance(op)
{
    m_matrixWidth = 5;
    m_radius = 1;
    m_gauss = 0.1;
    m_correlation = 0.5;
    m_noiseFactor = 0.01;
}

RefocusInstance::~RefocusInstance()
{
}

int RefocusInstance::getAttributeInt(std::string name)
{
    if (name == REFOCUS_ATTR_MATRIXSIZE)
    {
        return m_matrixWidth;
    }
}

double RefocusInstance::getAttributeDouble(std::string name)
{
    if (name == REFOCUS_ATTR_RADIUS)
    {
        return m_radius;
    }
    else if (name == REFOCUS_ATTR_GAUSS)
    {
        return m_gauss;
    }
    else if (name == REFOCUS_ATTR_CORRELATION)
    {
        return m_correlation;
    }
    else if (name == REFOCUS_ATTR_NOISE)
    {
        return m_noiseFactor;
    }
    else
    {
        printf("RefocusInstance::getAttribute: Unknown double attribute: %s\n", name.c_str());
    }
}


void RefocusInstance::setAttribute(std::string name, int i)
{
    printf("RefocusInstance::setAttribute: %s = %d\n", name.c_str(), i);
    if (name == REFOCUS_ATTR_MATRIXSIZE)
    {
        m_matrixWidth = i;
    }
    else
    {
        printf("RefocusInstance::setAttribute: Unknown int attribute: %s\n", name.c_str());
    }
}

void RefocusInstance::setAttribute(std::string name, double d)
{

printf("RefocusInstance::setAttribute: %s = %0.4f\n", name.c_str(), d);
    if (name == REFOCUS_ATTR_RADIUS)
    {
        m_radius = d;
    }
    else if (name == REFOCUS_ATTR_GAUSS)
    {
        m_gauss = d;
    }
    else if (name == REFOCUS_ATTR_CORRELATION)
    {
        m_correlation = d;
    }
    else if (name == REFOCUS_ATTR_NOISE)
    {
        m_noiseFactor = d;
    }
    else
    {
        printf("RefocusInstance::setAttribute: Unknown double attribute: %s\n", name.c_str());
    }
}

void RefocusInstance::apply(Surface* surface, ProgressListener* prog)
{
    // Circular
    CentredMatrix cmatrix(m_matrixWidth);
    CentredMatrix gmatrix(m_matrixWidth);

    make_circle_convolution(&cmatrix, m_radius);
    make_gaussian_convolution(&gmatrix, m_gauss);

    CentredMatrix conv(m_matrixWidth);
    conv.convolve_star_mat(&gmatrix, &cmatrix);

#if 0
    conv.dump();
    printf("\n");
#endif

    CentredMatrix* matrix;
    matrix = compute_g_matrix(
        &conv,
        m_matrixWidth,
        m_correlation,
        m_noiseFactor,
        0.0,
        true);

#if 0
    matrix->dump();
    printf("\n");
#endif

    // Apply the convolution
    Surface* dst = new Surface(surface->getWidth(), surface->getHeight(), 4);
    convolution(surface, dst, matrix);
    surface->swapData(dst);
    delete dst;
}

