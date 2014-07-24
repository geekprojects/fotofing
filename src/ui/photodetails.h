#ifndef __FOTOFING_UI_PHOTODETAILS_H_
#define __FOTOFING_UI_PHOTODETAILS_H_

#include <fotofing/photo.h>

#include <gtkmm.h>

class PhotoPropColumns : public Gtk::TreeModelColumnRecord
{
  public:
    Gtk::TreeModelColumn<Glib::ustring> property;
    Gtk::TreeModelColumn<Glib::ustring> value;

    PhotoPropColumns()
    {
        add(property);
        add(value);
    }
};

class PhotoTagColumns : public Gtk::TreeModelColumnRecord
{
  public:
    Gtk::TreeModelColumn<Glib::ustring> tag;

    PhotoTagColumns()
    {
        add(tag);
    }
};

class MainWindow;

class PhotoDetails : public Gtk::Paned
{
 private:
    MainWindow* m_mainWindow;

    // Photo properties
    Gtk::ScrolledWindow m_photoPropScrollWindow;
    Gtk::TreeView m_photoPropTreeView;
    Glib::RefPtr<Gtk::ListStore> m_photoPropListStore;
    const PhotoPropColumns m_photoPropColumns;
    Gtk::Frame m_photoPropFrame;

    // Photo tags
    Gtk::ScrolledWindow m_photoTagScrollWindow;
    Gtk::TreeView m_photoTagTreeView;
    Glib::RefPtr<Gtk::ListStore> m_photoTagListStore;
    const PhotoTagColumns m_photoTagColumns;
    Gtk::Frame m_photoTagFrame;

 public:

    PhotoDetails(MainWindow* mainWindow);
    ~PhotoDetails();

    void displayDetails(Photo* photo);
};

#endif
