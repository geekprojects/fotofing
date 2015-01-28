#ifndef __FOTOFING_WORKFLOW_H_
#define __FOTOFING_WORKFLOW_H_

#include <fotofing/photo.h>
#include <fotofing/file.h>
#include <fotofing/utils.h>
#include <fotofing/plugin.h>

#include <geek/core-matrix.h>

#include <vector>
#include <string>

class Workflow;
class OperationInstance;
class Index;

#define OPERATION_ATTR_TYPE_INT 1
#define OPERATION_ATTR_TYPE_DOUBLE 2

class WorkflowIndex
{
 private:
    Index* m_index;

 public:
    WorkflowIndex(Index* index);
    ~WorkflowIndex();

    Workflow* getWorkflow(Photo* photo);

    bool saveWorkflow(Workflow* w);
};

class Workflow
{
 private:
    int64_t m_id;
    Photo* m_photo;
    File* m_file;

    std::vector<OperationInstance*> m_operations;

 public:

    Workflow(Photo* photo, File* file)
    {
        m_id = -1;
        m_photo = photo;
        m_file = file;
    }

    Workflow(int64_t id, Photo* photo, File* file)
    {
        m_id = id;
        m_photo = photo;
        m_file = file;
    }

    ~Workflow()
    {
    }

    int64_t getId() { return m_id; }
    void setId(int64_t id) { m_id = id; }
    Photo* getPhoto() { return m_photo; }
    File* getFile() { return m_file; }

    void addOperation(OperationInstance* op);
    void deleteOperation(OperationInstance* op);

    std::vector<OperationInstance*>& getOperations() { return m_operations; }

    static bool init();
};

union AttributeValue
{
    int i;
    double d;
};

struct OperationAttribute
{
    std::string name;
    std::string label;
    std::string description;
    int type;

    AttributeValue min;
    AttributeValue max;
    AttributeValue def;

    bool operator < (const OperationAttribute& rhs) const
    {
        return name < rhs.name;
    }
};

class Operation : public FotofingPlugin
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
     Operation* m_operation;

    // Common utility methods
    void convolution(
        Geek::Gfx::Surface* src,
        Geek::Gfx::Surface* dst,
        Geek::CentredMatrix* matrix);

 public:
    OperationInstance(Operation* op);
    virtual ~OperationInstance();

    Operation* getOperation() { return m_operation; }

    virtual int getAttributeInt(std::string name);
    virtual double getAttributeDouble(std::string name);
    virtual void setAttribute(std::string name, int i);
    virtual void setAttribute(std::string name, double d);

    virtual void apply(
        Geek::Gfx::Surface* surface,
        ProgressListener* prog);

};

typedef Operation*(*newOperation_t)();

#define DECLARE_OPERATION(_class) DECLARE_PLUGIN(operation, _class)

#endif
