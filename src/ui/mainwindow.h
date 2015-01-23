#ifndef __FOTOFING_WINDOW_H_
#define __FOTOFING_WINDOW_H_

#include <map>
#include <string>

#include <gtkmm.h>

#include <fotofing/index.h>
#include <fotofing/workflow.h>

#include "dialogs/about.h"
#include "library/library.h"

class MainWindow : public Gtk::Window, public IndexClient
{
 private:
    Gtk::VBox m_vBox;

    Glib::RefPtr<Gtk::Builder> m_refBuilder;

    Gtk::Notebook m_tabs;

    /* Library Tab */
    Library m_library;

    /* Workflow */
    WorkflowIndex* m_workflowIndex;

    /* *** Status Bar *** */
    Gtk::HBox m_statusBox;
    Gtk::Statusbar m_statusBar;
    Gtk::ProgressBar m_progressBar;
    bool m_progressActive;
    bool progressTimeout();

    About m_about;

    Gtk::MenuBar* createMenu();

 public:
    MainWindow(Index* index);
    virtual ~MainWindow();

    Index* getIndex() { return m_index; }
    WorkflowIndex* getWorkflowIndex() { return m_workflowIndex; }

    void editPhoto(Photo* photo);

    void setStatusMessage(std::string message);
    void startProgress();
    void endProgress();
    void updateProgress(
        int complete,
        int total,
        std::string info);
};

#endif
