#ifndef __FOTOFING_UI_PHOTOVIEW2_H_
#define __FOTOFING_UI_PHOTOVIEW2_H_

#include <vector>

#include <gtkmm.h>

#include <fotofing/photo.h>
#include "widgets/tagview.h"

class Library;

struct PhotoIcon
{
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

class PhotoView2 : public Gtk::DrawingArea, public Gtk::Scrollable
{
 private:
    Library* m_library;

    std::vector<PhotoIcon*> m_photos;
    std::vector<PhotoIcon*>::iterator m_photoCursor;

    int m_padding;
    int m_margin;
    int m_titleHeight;
    int m_columns;

    int m_maxThumbWidth;

    Gtk::Menu m_popupMenu;

    void clearPhotos();

    PhotoIcon* getIcon(double x, double y);

    Gtk::Viewport* getViewport() const;
    Glib::RefPtr<Gtk::Adjustment> getScrollAdjustment();
    int getScrollHeight() const;

    void moveCursor(PhotoIcon* icon);
    void moveCursor(std::vector<PhotoIcon*>::iterator pos);
    void moveCursor(int a, bool select);
    void movePage(int a, bool selectPage);
    void updateCursor();
    void scrollToCursor();
    void scrollToIcon(PhotoIcon* icon);

 protected:

    bool onPopupMenu();

 public:
    PhotoView2(Library* library);
    virtual ~PhotoView2();

    void update(std::vector<Tag*> tags, time_t from, time_t to);

    void clearSelection();
    std::vector<Photo*> getSelectedPhotos();

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
};

#endif
