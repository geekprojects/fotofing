#ifndef __FOTOFING_WORKFLOW_H_
#define __FOTOFING_WORKFLOW_H_

#include <fotofing/photo.h>
#include <fotofing/file.h>
#include <fotofing/utils.h>

#include <geek/core-matrix.h>

#include <vector>
#include <string>

class OperationInstance;

class Workflow
{
 private:
    Photo* m_photo;
    File* m_file;

    std::vector<OperationInstance*> m_operations;

 public:
    Workflow(Photo* photo, File* file)
    {
        m_photo = photo;
        m_file = file;
    }

    ~Workflow()
    {
    }

    Photo* getPhoto() { return m_photo; }
    File* getFile() { return m_file; }
};

struct OperationAttribute
{
    std::string name;
    std::string label;
    std::string description;
    int type;
};

class Operation
{
 public:
    Operation();
    virtual ~Operation();

    virtual std::string getName();
    virtual std::string getDescription();

    virtual std::vector<OperationAttribute> getAttributes();

    virtual OperationInstance* createInstance();
};

class OperationInstance
{
 protected:

    // Common utility methods
    void convolution(
        Geek::Gfx::Surface* src,
        Geek::Gfx::Surface* dst,
        Geek::CentredMatrix* matrix);

 public:
    OperationInstance();
    virtual ~OperationInstance();

    virtual void setAttribute(std::string name, int i);
    virtual void setAttribute(std::string name, double d);

    virtual void apply(
        Geek::Gfx::Surface* surface,
        ProgressListener* prog);

};

typedef Operation*(*newOperation_t)();

#define DECLARE_OPERATION(_class) \
    extern "C" Operation* fotofing_operation_create() \
    { \
        return new _class(); \
    }

#endif
