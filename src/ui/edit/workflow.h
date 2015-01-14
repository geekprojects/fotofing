#ifndef __FOTOFING_WORKFLOW_H_
#define __FOTOFING_WORKFLOW_H_

/* This is just a placeholder for now */

#include <fotofing/photo.h>
#include <fotofing/file.h>

class Workflow
{
 private:
    Photo* m_photo;
    File* m_file;

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

#endif
