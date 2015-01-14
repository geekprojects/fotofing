#ifndef __FOTOFING_UI_EDIT_EDIT_H_
#define __FOTOFING_UI_EDIT_EDIT_H_

#include <gtkmm.h>

#include <fotofing/photo.h>

#include "mainwindow.h"
#include "editpreview.h"
#include "workflow.h"

class Edit : public Gtk::VBox
{
 private:
    MainWindow* m_mainWindow;

    Workflow* m_workflow;

    Gtk::Label m_label;
    EditPreview m_preview;

    Gtk::HBox m_tabLabel;
    Gtk::Label m_tabLabelText;
    Gtk::Button m_tabLabelClose;

 public:
    Edit(MainWindow* mainWindow, Photo* photo);
    ~Edit();

    Gtk::Widget* getTabLabel() { return &m_tabLabel; }
};

#endif
