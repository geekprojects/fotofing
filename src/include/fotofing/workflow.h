#ifndef __FOTOFING_WORKFLOW_H_
#define __FOTOFING_WORKFLOW_H_

#include <fotofing/photo.h>
#include <fotofing/file.h>

#include <vector>

class WorkflowOperation;

class Workflow
{
 private:
    Photo* m_photo;
    File* m_file;

    std::vector<WorkflowOperation*> m_operations;

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

class WorkflowOperation
{
 private:

 public:
    WorkflowOperation();
    virtual ~WorkflowOperation();

    virtual void apply(Geek::Gfx::Surface* surface);
};

#endif
