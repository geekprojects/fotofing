#ifndef __FOTOFING_UI_PHOTODETAILS_H_
#define __FOTOFING_UI_PHOTODETAILS_H_

#include <fotofing/photo.h>

#include <gtkmm.h>

#include "tagview.h"

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

class MainWindow;

class PhotoDetails : public Gtk::Paned
{
 private:
    MainWindow* m_mainWindow;
    Photo* m_photo;

    // Photo properties
    Gtk::ScrolledWindow m_photoPropScrollWindow;
    Gtk::TreeView m_photoPropTreeView;
    Glib::RefPtr<Gtk::ListStore> m_photoPropListStore;
    const PhotoPropColumns m_photoPropColumns;
    Gtk::Frame m_photoPropFrame;

    // Photo tags
    TagView m_tagView;
    Gtk::Frame m_photoTagFrame;

    void onDeleteTags(std::vector<Tag*> tags);

 public:

    PhotoDetails(MainWindow* mainWindow);
    ~PhotoDetails();

    void displayDetails(Photo* photo);

    void updateTags();
};

#endif
