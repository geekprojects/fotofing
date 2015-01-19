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
    Operation();
    virtual ~Operation();

    virtual std::string getName();
    virtual std::string getDescription();

    virtual std::vector<OperationAttribute> getAttributes();
};

class OperationInstance
{
 private:

 public:
    OperationInstance();
    virtual ~OperationInstance();

    virtual void setAttribute(std::string name, int i);
    virtual void setAttribute(std::string name, double d);

    virtual void apply(
        Geek::Gfx::Surface* surface,
        ProgressListener* prog);

    // Common utility methods
    void convolution(Geek::Gfx::Surface* surface, Geek::CentredMatrix* matrix);
};

#define DECLARE_OPERATION(_class) \
    Operation* fotofing_operator_new() \
    { \
        return new _class(); \
    }

#endif
