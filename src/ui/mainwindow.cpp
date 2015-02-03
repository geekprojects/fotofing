
#include <fotofing/utils.h>

#include "dialogs/calendarpopup.h"
#include "dialogs/sourcesdialog.h"
#include "dialogs/tagdialog.h"
#include "edit/edit.h"
#include "mainwindow.h"
#include "uiutils.h"

using namespace std;

MainWindow::MainWindow(Index* index) :
    m_library(this)
{
    m_index = index;
    m_workflowIndex = new WorkflowIndex(index);

    openTab(&m_library);

    initActions();

    m_library.updateSources();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initActions()
{
    Glib::RefPtr<Gio::SimpleActionGroup> actionGroup;
    actionGroup = Gio::SimpleActionGroup::create();
    actionGroup->add_action(
        "about",
        sigc::mem_fun(m_about, &About::open));

    actionGroup->add_action(
        "sources",
        sigc::mem_fun(m_library, &Library::openSourcesDialog));

    actionGroup->add_action(
        "edit_export",
        sigc::mem_fun(this, &MainWindow::onExport));

/*
    actionGroup->add_action(
        "selectAll",
        sigc::mem_fun(m_photoView, &PhotoView::selectAll));
*/

    insert_action_group("fotofing", actionGroup);
}

void MainWindow::editPhoto(Photo* photo)
{
    Workflow* workflow = m_workflowIndex->getWorkflow(photo);
    Edit* editTab = (Edit*)Gtk::manage(new Edit(this, workflow));

    openTab(editTab);
}

void MainWindow::onExport()
{
    Edit* editTab = (Edit*)getCurrentTab();
    editTab->onExport();
}

