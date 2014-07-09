#ifndef __FOTOFING_WINDOW_H_
#define __FOTOFING_WINDOW_H_

#include <gtkmm.h>

#include "index.h"

class ModelColumns : public Gtk::TreeModelColumnRecord
{
  public:

    Gtk::TreeModelColumn<Glib::ustring> display_name;
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > pixbuf;

    ModelColumns()
    {
        add(display_name);
        add(pixbuf);
    }
};


class MainWindow : public Gtk::Window
{
 private:
    Index* m_index;

    const ModelColumns m_columns;

    Gtk::ScrolledWindow m_scrolledWindow;

    Gtk::VBox m_vBox;
    Gtk::IconView m_iconView;
    Glib::RefPtr<Gtk::ListStore> m_model;

 public:
    MainWindow(Index* index);
    virtual ~MainWindow();

    void update();
};

#endif
