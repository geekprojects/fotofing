#ifndef __FOTOFING_REFOCUS_REFOCUS_H_
#define __FOTOFING_REFOCUS_REFOCUS_H_

#include <fotofing/workflow.h>

class RefocusInstance;

#define REFOCUS_ATTR_MATRIXSIZE "matrixSize"
#define REFOCUS_ATTR_RADIUS "radius"
#define REFOCUS_ATTR_GAUSS "gauss"
#define REFOCUS_ATTR_CORRELATION "correlation"
#define REFOCUS_ATTR_NOISE "noise"

class RefocusOperation : public Operation
{
 private:
 public:
    RefocusOperation();
    virtual ~RefocusOperation();

    std::string getName() { return "Refocus"; }
    std::string getDescription() { return "Refocus with FIR Wiener Deconvolution"; }

    std::vector<OperationAttribute> getAttributes();

    OperationInstance* createInstance();
};

class RefocusInstance : public OperationInstance
{
 private:
    int m_matrixWidth;
    double m_radius;
    double m_gauss;
    double m_correlation;
    double m_noiseFactor;

 public:
    RefocusInstance(RefocusOperation* op);
    virtual ~RefocusInstance();

    int getAttributeInt(std::string name);
    double getAttributeDouble(std::string name);
    void setAttribute(std::string name, int i);
    void setAttribute(std::string name, double d);

    virtual void apply(
        Geek::Gfx::Surface* surface,
        ProgressListener* prog);

};

#endif
