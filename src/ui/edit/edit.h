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

    OpsModelColumns()
    {
        add(opText);
    }
};


class Edit : public Gtk::HBox
{
 private:
    MainWindow* m_mainWindow;

    Workflow* m_workflow;

    EditPreview m_preview;

    Gtk::VBox m_opsBox;

    Gtk::ScrolledWindow m_opsMenuWindow;
    Gtk::Frame m_opsMenuFrame;
    OpsModelColumns m_opsMenuColumns;
    Gtk::TreeView m_opsMenuView;
    Glib::RefPtr<Gtk::TreeStore> m_opsMenuViewStore;

    Gtk::ScrolledWindow m_opsWindow;
    Gtk::Frame m_opsFrame;
    OpsModelColumns m_opsColumns;
    Gtk::TreeView m_opsView;
    Glib::RefPtr<Gtk::TreeStore> m_opsViewStore;

    Gtk::HBox m_tabLabel;
    Gtk::Label m_tabLabelText;
    Gtk::Button m_tabLabelClose;

 public:
    Edit(MainWindow* mainWindow, Photo* photo);
    ~Edit();

    Gtk::Widget* getTabLabel() { return &m_tabLabel; }
};

#endif
