#ifndef __FOTOFING_UI_PHOTOVIEW2_H_
#define __FOTOFING_UI_PHOTOVIEW2_H_

#include <vector>

#include <gtkmm.h>

#include <fotofing/photo.h>
#include "widgets/tagview.h"

class Library;
class Index;
class PhotoView;

enum PhotoViewSort
{
    PHOTOVIEW_SORT_TIMESTAMP,
    PHOTOVIEW_SORT_TITLE,
};

struct PhotoIcon
{
    PhotoView* photoView;
    int x;
    int y;
    int width;
    int height;
    int maxRowThumbHeight;

    Photo* photo;
    bool selected;

    std::string title;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;
};

class PhotoView : public Gtk::DrawingArea, public Gtk::Scrollable
{
 private:
    Index* m_index;

    std::vector<PhotoIcon*> m_photos;
    std::vector<PhotoIcon*>::iterator m_photoCursor;

    int m_padding;
    int m_margin;
    int m_titleHeight;
    int m_columns;

    int m_maxThumbWidth;

    PhotoViewSort m_sortBy;
    bool m_sortDir;

    Gtk::Menu m_popupMenu;

    void clearPhotos();

    PhotoIcon* getIcon(double x, double y);

    Gtk::Viewport* getViewport() const;
    Glib::RefPtr<Gtk::Adjustment> getScrollAdjustment();
    int getScrollHeight() const;

    sigc::signal<void, Photo*> m_cursorChangedSignal;
    sigc::signal<void, Photo*> m_activatePhotoSignal;

    void moveCursor(PhotoIcon* icon);
    void moveCursor(std::vector<PhotoIcon*>::iterator pos);
    void moveCursor(int a, bool select);
    void movePage(int a, bool selectPage);
    void updateCursor();
    void scrollToCursor();
    void scrollToIcon(PhotoIcon* icon);

    void sort();

 protected:

    bool onPopupMenu();

 public:
    PhotoView(Index* index);
    virtual ~PhotoView();

    void update(std::vector<Photo*> photos);

    void clearSelection();
    std::vector<Photo*> getSelectedPhotos();

    void setSort(PhotoViewSort sortBy, bool direction);
    PhotoViewSort getSortBy() { return m_sortBy; }
    bool getSortDir() { return m_sortDir; }

    virtual Gtk::SizeRequestMode get_request_mode_vfunc() const;
    virtual void get_preferred_height_for_width_vfunc(
        int width,
        int& minimum_height,
        int& natural_height) const;
    virtual void on_size_allocate(Gtk::Allocation& allocation);
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

    virtual bool on_button_press_event(GdkEventButton* event);
    virtual bool on_key_press_event(GdkEventKey* event);

    void rename();

    sigc::signal<void, Photo*>& signal_cursor_changed();
    sigc::signal<void, Photo*>& signal_activate_photo();

};

#endif
