#ifndef __FOTOFING_REFOCUS_REFOCUS_H_
#define __FOTOFING_REFOCUS_REFOCUS_H_

#include <fotofing/workflow.h>

class RefocusInstance;

class RefocusOperation : public Operation
{
 private:

 public:
    RefocusOperation();
    virtual ~RefocusOperation();

    std::string getName() { return "Refocus"; }
    std::string getDescription() { return "Refocus with FIR Wiener Deconvolution"; }

    OperationInstance* createInstance();
};

class RefocusInstance : public OperationInstance
{
 private:

 public:
    RefocusInstance();
    virtual ~RefocusInstance();

    virtual void apply(
        Geek::Gfx::Surface* surface,
        ProgressListener* prog);

};

#endif
