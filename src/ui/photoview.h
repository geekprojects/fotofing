#ifndef __FOTOFING_UI_PHOTOVIEW_H_
#define __FOTOFING_UI_PHOTOVIEW_H_

#include <vector>

#include <gtkmm.h>

#include <fotofing/photo.h>

#include "tagview.h"

class MainWindow;

class PhotoModelColumns : public Gtk::TreeModelColumnRecord
{
  public:

    Gtk::TreeModelColumn<Glib::ustring> display_name;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > pixbuf;
    Gtk::TreeModelColumn<Photo*> photo;
    Gtk::TreeModelColumn<time_t> timestamp;

    PhotoModelColumns()
    {
        add(display_name);
        add(pixbuf);
        add(photo);
        add(timestamp);
    }
};

class PhotoView : public Gtk::ScrolledWindow
{
 private:
    MainWindow* m_mainWindow;

    std::vector<Photo*> m_photos;
    const PhotoModelColumns m_photoColumns;
    Gtk::IconView m_iconView;
    Glib::RefPtr<Gtk::ListStore> m_model;

    void onIconViewItemActivated(const Gtk::TreeModel::Path& path);
    void onIconViewSelectionChanged();
    int onIconViewSort(const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);

    Photo* getPhotoFromPath(Gtk::TreePath path);

    void freePhotos();

 public:
    PhotoView(MainWindow* mainWindow);
    ~PhotoView();

    void update(std::vector<Tag*> tags, time_t from, time_t to);

    std::vector<Photo*> getSelectedPhotos();
};

#endif
