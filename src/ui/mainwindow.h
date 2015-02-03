#ifndef __FOTOFING_UI_MAINWINDOW_H_
#define __FOTOFING_UI_MAINWINDOW_H_

#include <map>
#include <string>

#include <gtkmm.h>

#include <fotofing/index.h>
#include <fotofing/workflow.h>

#include "window.h"
#include "dialogs/about.h"
#include "library/library.h"

class MainWindow : public FotofingWindow
{
 private:
    /* Library Tab */
    Library m_library;

    /* Workflow */
    WorkflowIndex* m_workflowIndex;

    About m_about;

    void initActions();

 public:
    MainWindow(Index* index);
    virtual ~MainWindow();

    Index* getIndex() { return m_index; }
    WorkflowIndex* getWorkflowIndex() { return m_workflowIndex; }

    void editPhoto(Photo* photo);

    void onExport();
};

#endif
