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

    PhotoModelColumns()
    {
        add(display_name);
        add(pixbuf);
        add(photo);
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

    Gtk::Menu m_popupMenu;

    void onIconViewItemActivated(const Gtk::TreeModel::Path& path);
    void onIconViewSelectionChanged();
    int onIconViewSort(const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);
    bool onButtonPress(GdkEventButton* event);

    Photo* getPhotoFromPath(Gtk::TreePath path);

    void freePhotos();

 public:
    PhotoView(MainWindow* mainWindow);
    ~PhotoView();

    void update(std::vector<Tag*> tags, time_t from, time_t to);

    void selectAll();

    void addTag();
    void rename();

    std::vector<Photo*> getSelectedPhotos();
};

#endif
