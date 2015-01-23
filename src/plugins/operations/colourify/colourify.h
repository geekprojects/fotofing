#ifndef __FOTOFING_COLOURIFY_COLOURIFY_H_
#define __FOTOFING_COLOURIFY_COLOURIFY_H_

#include <fotofing/workflow.h>

class ColourifyInstance;

class ColourifyOperation : public Operation
{
 private:

 public:
    ColourifyOperation();
    virtual ~ColourifyOperation();

    std::string getName() { return "Colourify"; }
    std::string getDescription() { return "Replace all colours with a single colour"; }

    OperationInstance* createInstance();
};

class ColourifyInstance : public OperationInstance
{
 private:

 public:
    ColourifyInstance(ColourifyOperation* op);
    virtual ~ColourifyInstance();

    virtual void apply(
        Geek::Gfx::Surface* surface,
        ProgressListener* prog);

};

#endif
