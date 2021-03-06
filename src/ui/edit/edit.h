#ifndef __FOTOFING_UI_EDIT_EDIT_H_
#define __FOTOFING_UI_EDIT_EDIT_H_

#include <gtkmm.h>

#include <fotofing/photo.h>
#include <fotofing/workflow.h>

#include "mainwindow.h"
#include "tab.h"
#include "editpreview.h"

class OpsModelColumns : public Gtk::TreeModelColumnRecord
{
 public:
    Gtk::TreeModelColumn<Glib::ustring> opText;
    Gtk::TreeModelColumn<Operation*> operation;
    Gtk::TreeModelColumn<OperationAttribute> attrs;

    OpsModelColumns()
    {
        add(opText);
        add(operation);
        add(attrs);
    }
};


class Edit : public Tab
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

    Glib::RefPtr<Gio::Menu> m_menu;
    Gtk::HBox m_tabLabel;
    Gtk::Label m_tabLabelText;
    Gtk::Button m_tabLabelClose;

    void createMenu();

    void updateOperations();
    void updateWorkflow();

 public:
    Edit(MainWindow* mainWindow, Workflow* workflow);
    ~Edit();

    void close();

    Gtk::Widget* getTabLabel() { return &m_tabLabel; }
    Glib::RefPtr< Gio::Menu >& getMenu(){ return m_menu; }

    MainWindow* getMainWindow() { return m_mainWindow; }

    void deleteOperation(OperationInstance* op);

    void onExport();

    void onOpsMenuRowActivate(
        const Gtk::TreeModel::Path& path,
        Gtk::TreeViewColumn* column);

    void onAttrChanged();

    bool isVisible() { return m_mainWindow->isTabVisible(this); }
};

#endif
