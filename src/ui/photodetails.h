#ifndef __FOTOFING_UI_PHOTODETAILS_H_
#define __FOTOFING_UI_PHOTODETAILS_H_

#include <fotofing/photo.h>

#include <gtkmm.h>

#include "tagview.h"
#include "histogramwidget.h"

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
class Library;

class PhotoDetails : public Gtk::Paned
{
 private:
    Library* m_library;
    Photo* m_photo;

    Gtk::VBox m_vbox;

    // Photo properties
    Gtk::ScrolledWindow m_photoPropScrollWindow;
    Gtk::TreeView m_photoPropTreeView;
    Glib::RefPtr<Gtk::ListStore> m_photoPropListStore;
    const PhotoPropColumns m_photoPropColumns;
    Gtk::Frame m_photoPropFrame;

    HistogramWidget m_histogram;

    // Photo tags
    TagView m_tagView;
    Gtk::Frame m_photoTagFrame;

    void onDeleteTags(std::vector<Tag*> tags);

 public:

    PhotoDetails(Library* library);
    ~PhotoDetails();

    void displayDetails(Photo* photo);

    void updateTags();
};

#endif
