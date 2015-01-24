#ifndef __FOTOFING_UI_EDIT_EDIT_H_
#define __FOTOFING_UI_EDIT_EDIT_H_

#include <gtkmm.h>

#include <fotofing/photo.h>
#include <fotofing/workflow.h>

#include "mainwindow.h"
#include "editpreview.h"

class OpsModelColumns : public Gtk::TreeModelColumnRecord
{
 public:
    Gtk::TreeModelColumn<Glib::ustring> opText;
    Gtk::TreeModelColumn<Operation*> operation;

    OpsModelColumns()
    {
        add(opText);
        add(operation);
    }
};


class Edit : public Gtk::HBox
{
 private:
    MainWindow* m_mainWindow;

    Workflow* m_workflow;

    EditPreview m_preview;

    Gtk::VBox m_workflowBox;

    Gtk::ScrolledWindow m_opsMenuWindow;
    Gtk::Frame m_opsMenuFrame;
    OpsModelColumns m_opsMenuColumns;
    Gtk::TreeView m_opsMenuView;
    Glib::RefPtr<Gtk::TreeStore> m_opsMenuViewStore;
    Glib::RefPtr<Gtk::TreeSelection> m_opsMenuSelection;

    Gtk::VBox m_opsBox;
    Gtk::ScrolledWindow m_opsWindow;
    Gtk::Frame m_opsFrame;
#if 0
    OpsModelColumns m_opsColumns;
    Gtk::TreeView m_opsView;
    Glib::RefPtr<Gtk::TreeStore> m_opsViewStore;
#endif

    Gtk::HBox m_tabLabel;
    Gtk::Label m_tabLabelText;
    Gtk::Button m_tabLabelClose;

    void updateOperations();
    void updateWorkflow();

 public:
    Edit(MainWindow* mainWindow, Workflow* workflow);
    ~Edit();

    void close();

    Gtk::Widget* getTabLabel() { return &m_tabLabel; }

    void onOpsMenuRowActivate(
        const Gtk::TreeModel::Path& path,
        Gtk::TreeViewColumn* column);

    void onAttrAdjustmentChanged();

    bool isVisible() { return m_mainWindow->isTabVisible(this); }
};

#endif
