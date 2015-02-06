
#include "library/photoview2.h"
#include "library/library.h"
#include "mainwindow.h"

using namespace std;
using namespace Geek::Gfx;

PhotoView2::PhotoView2(Library* library)
{
    m_library = library;
    override_background_color(Gdk::RGBA("#101010"));

    m_padding = 20;
    m_margin = 0;
    m_titleHeight = 20;

    set_can_focus(true);
    set_vexpand(true);
    set_vscroll_policy(Gtk::SCROLL_NATURAL);

    set_events(Gdk::BUTTON_PRESS_MASK | Gdk::KEY_PRESS_MASK);
}

PhotoView2::~PhotoView2()
{
}

void PhotoView2::update(vector<Tag*> tags, time_t from, time_t to)
{
    clearPhotos();

    vector<Photo*> photos;
    if (tags.size() > 0)
    {
        vector<string> tagStrings;
        vector<Tag*>::iterator it;
        for (it = tags.begin(); it != tags.end(); it++)
        {
            tagStrings.push_back((*it)->getTagName());
        }
        photos = m_library->getIndex()->getPhotos(tagStrings, &from, &to);
    }
    else
    {
        photos = m_library->getIndex()->getPhotos(&from, &to);
    }

    m_maxThumbWidth = 0;

    vector<Photo*>::iterator it;
    for (it = photos.begin(); it != photos.end(); it++)
    {
        PhotoIcon* icon = new PhotoIcon();
        icon->photo = *it;
#if 0
        icon->selected = (it == photos.begin());
        if (icon->selected)
        {
            m_library->displayDetails(icon->photo);
        }
#else
        icon->selected = false;
#endif

        Surface* thumb = icon->photo->getThumbnail();
        icon->pixbuf = Gdk::Pixbuf::create_from_data(
            thumb->getData(),
            Gdk::COLORSPACE_RGB,
            true,
            8,
            thumb->getWidth(),
            thumb->getHeight() - 1,
            thumb->getWidth() * 4);

        if (thumb->getWidth() > m_maxThumbWidth)
        {
            m_maxThumbWidth = thumb->getWidth();
        }

        m_photos.push_back(icon);
    }

    queue_resize();
}

Gtk::SizeRequestMode PhotoView2::get_request_mode_vfunc() const
{
    return Gtk::SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

/*
 * Figure out our height given a width. This involves taking the width
 * and seeing how many photos we can fit on a row, then finding out how
 * many rows we need, and their heights
 */
void PhotoView2::get_preferred_height_for_width_vfunc(
    int width,
    int& minimum_height,
    int& natural_height) const
{
    int iconWidth = m_maxThumbWidth + (m_padding * 2) + m_margin;

    int maxCols = width / iconWidth;

    int rows = width / maxCols;
    if (width % maxCols > 0)
    {
        rows++;
    }

    int row = 0;

    printf(
        "PhotoView2::get_preferred_height_for_width: maxCols=%d (%d / %d)\n",
        maxCols,
        width,
        iconWidth);


    int y = m_margin;
    vector<PhotoIcon*>::const_iterator it;
    for (it = m_photos.begin();
        it != m_photos.end();)
    {
        int col = 0;

        int maxThumbHeight = 0;
        for (col = 0; it != m_photos.end() && col < maxCols; it++, col++)
        {
            PhotoIcon* icon = *it;
            Surface* thumb = icon->photo->getThumbnail();

            if (thumb->getHeight() > maxThumbHeight)
            {
                maxThumbHeight = thumb->getHeight();
            }
        }
        y += maxThumbHeight + m_titleHeight + (m_padding * 2) + m_margin;
    }
    printf("PhotoView2::get_preferred_height_for_width: y=%d\n", y);
    minimum_height = y;
    natural_height = y;
}

/*
 * We give each photo a position once we have a size. This saves
 * us having to do it too often.
 */
void PhotoView2::on_size_allocate(Gtk::Allocation& allocation)
{
    Gtk::DrawingArea::on_size_allocate(allocation);

    const int width = allocation.get_width();
    const int height = allocation.get_height();
    int iconWidth = m_maxThumbWidth + (m_padding * 2) + m_margin;

    m_columns = width / iconWidth;

    int rows = width / m_columns;
    if (width % m_columns > 0)
    {
        rows++;
    }

    int row = 0;

    printf(
        "PhotoView2::on_size_allocate: width=%d, height=%d\n",
        width,
        height);
    printf(
        "PhotoView2::on_size_allocate: m_columns=%d (%d / %d)\n",
        m_columns,
        width,
        iconWidth);
    printf(
        "PhotoView2::on_size_allocate: rows=%d\n",
        rows);


    int y = m_margin;
    vector<PhotoIcon*>::iterator it;
    for (it = m_photos.begin(); it != m_photos.end();)
    {
        int col = 0;
        int x = m_margin;
        int maxThumbHeight = 0;

        vector<PhotoIcon*> iconRow;
        for (col = 0; it != m_photos.end() && col < m_columns; it++, col++)
        {
            PhotoIcon* icon = *it;
            Surface* thumb = icon->photo->getThumbnail();

            icon->x = x;
            icon->y = y;
            icon->width = m_padding + m_maxThumbWidth + m_padding;
            x += icon->width;
            x += m_margin;

            if (thumb->getHeight() > maxThumbHeight)
            {
                maxThumbHeight = thumb->getHeight();
            }
            iconRow.push_back(icon);
        }

        vector<PhotoIcon*>::iterator rowIt;
        for (rowIt = iconRow.begin(); rowIt != iconRow.end(); rowIt++)
        {
            PhotoIcon* icon = *rowIt;
            icon->height = m_padding + maxThumbHeight + m_titleHeight + m_padding;
            icon->maxRowThumbHeight = maxThumbHeight;
        }

        row++;
        x = 0;
        y += maxThumbHeight + m_titleHeight + (m_padding * 2) + m_margin;
    }
}

bool PhotoView2::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{


  Pango::FontDescription font;

  font.set_family("Monospace");
  font.set_weight(Pango::WEIGHT_BOLD);


    double clipX1;
    double clipY1;
    double clipX2;
    double clipY2;

    cr->get_clip_extents(clipX1, clipY1, clipX2, clipY2);
printf("PhotoView2::on_draw: clipY1=%0.2f\n", clipY1);

    int skipped = 0;

    vector<PhotoIcon*>::iterator it;
    for (it = m_photos.begin(); it != m_photos.end(); it++)
    {
        PhotoIcon* icon = *it;
        Surface* thumb = icon->photo->getThumbnail();

        // Check that the icon is actually viewable
        if ((icon->y < clipY1 - icon->height) ||
            (icon->y > clipY2 + icon->height))
        {
            skipped++;
            continue;
        }

        int photoX = icon->x + ((icon->width / 2) - (thumb->getWidth() / 2));
        int photoY = icon->y + ((icon->height / 2) - (thumb->getHeight() / 2));

        cr->set_line_width(1.0);

        if (icon->selected)
        {
            cr->set_source_rgb(0.5, 0.5, 0.5);    // partially translucent

        cr->rectangle(
            icon->x,
            icon->y,
            icon->width,
            icon->height);
            cr->fill_preserve();
        }
        cr->stroke();

        cr->set_source_rgb(0, 0, 0);

        cr->rectangle(
            icon->x,
            icon->y,
            icon->width,
            icon->height);
        cr->stroke();

        cr->set_source_rgb(0, 0, 0);

        // Border around the photo itself
        cr->rectangle(
            photoX - 0,
            photoY - 0,
            thumb->getWidth() + 1,
            thumb->getHeight() + 1);
        cr->stroke();

        Glib::RefPtr<Pango::Layout> layout = create_pango_layout(icon->photo->getId().substr(0, 6));
        //layout->set_font_description(font);

        int text_width;
        int text_height;
        layout->get_pixel_size(text_width, text_height);

        int textX = icon->x + ((icon->width / 2) - (text_width / 2));
        int textY = icon->y + m_padding + (text_height / 2) + icon->maxRowThumbHeight + 5;

        cr->set_source_rgb(0, 0, 0);
        cr->move_to(textX + 1, textY + 1);
        layout->show_in_cairo_context(cr);

        cr->set_source_rgb(1, 1, 1);
        cr->move_to(textX, textY);
        layout->show_in_cairo_context(cr);

        Gdk::Cairo::set_source_pixbuf(
            cr,
            icon->pixbuf,
            photoX,
            photoY);
        cr->paint();
    }
    printf("PhotoView2::on_draw: Skipped=%d\n", skipped);

    return true;
}

bool PhotoView2::on_button_press_event(GdkEventButton* event)
{
    printf("PhotoView2::on_button_press_event: x=%0.2f, y=%0.2f\n", event->x, event->y);

    grab_focus();

    PhotoIcon* selected = NULL;

    vector<PhotoIcon*>::iterator it;
    for (it = m_photos.begin(); it != m_photos.end(); it++)
    {
        PhotoIcon* icon = *it;
        if (event->x >= icon->x &&
            event->y >= icon->y &&
            event->x < icon->x + icon->width  &&
            event->y < icon->y + icon->height)
        {
            selected = icon;
            m_photoCursor = it;
            break;
        }
    }

    if (selected != NULL)
    {
        if (!selected->selected)
        {
            clearSelection();

            selected->selected = true;
            queue_draw();

            m_library->displayDetails(selected->photo);
        }
        else if (event->type == GDK_2BUTTON_PRESS)
        {
            printf("PhotoView2::on_button_press_event: Double click!\n");
            m_library->getMainWindow()->editPhoto(selected->photo);
        }
    }

    return true;
}

bool PhotoView2::on_key_press_event(GdkEventKey* event)
{
    printf("PhotoView2::on_key_press_event: key=0x%x\n", event->keyval);

    switch (event->keyval)
    {
        case GDK_KEY_Left:
            moveCursor(-1);
            break;

        case GDK_KEY_Right:
            moveCursor(1);
            break;

        case GDK_KEY_Up:
            moveCursor(-m_columns);
            break;

        case GDK_KEY_Home:
            moveCursor(m_photos.begin());
            break;

        case GDK_KEY_End:
            moveCursor(m_photos.end() - 1);
            break;

        case GDK_KEY_Down:
            moveCursor(m_columns);
            break;

        case GDK_KEY_Page_Up:
            movePage(-1);
            break;

        case GDK_KEY_Page_Down:
            movePage(1);
            break;

        case GDK_KEY_Return:
            m_library->getMainWindow()->editPhoto((*m_photoCursor)->photo);
            break;
    }
    return true;
}

vector<Photo*> PhotoView2::getSelectedPhotos()
{
    vector<Photo*> results;
    return results;
}

void PhotoView2::clearPhotos()
{
    vector<PhotoIcon*>::iterator it;
    for (it = m_photos.begin(); it != m_photos.end(); it++)
    {
        PhotoIcon* icon = *it;
        delete icon->photo;
        delete icon;
    }
    m_photos.clear();
    m_photoCursor = m_photos.end();
}

void PhotoView2::clearSelection()
{
    vector<PhotoIcon*>::iterator it;
    for (it = m_photos.begin(); it != m_photos.end(); it++)
    {
        PhotoIcon* icon = *it;
        icon->selected = false;
    }
}

PhotoIcon* PhotoView2::getIcon(double x, double y)
{
    PhotoIcon* selected = NULL;

    vector<PhotoIcon*>::iterator it;
    for (it = m_photos.begin(); it != m_photos.end(); it++)
    {
        PhotoIcon* icon = *it;
        if (x >= icon->x &&
            y >= icon->y &&
            x < icon->x + icon->width  &&
            y < icon->y + icon->height)
        {
            selected = icon;
            break;
        }
    }
    return selected;
}

Gtk::Viewport* PhotoView2::getViewport() const
{
    const Gtk::Container* parent = get_parent();
    if (parent == NULL || !GTK_IS_VIEWPORT(parent->gobj()))
    {
        printf("PhotoView2::getViewport: Parent is not a viewport! (%p)\n", parent);
        return NULL;
    }
    return (Gtk::Viewport*)parent;

}

Glib::RefPtr<Gtk::Adjustment> PhotoView2::getScrollAdjustment()
{
    Gtk::Viewport* parent = getViewport();
    return parent->get_vadjustment();
}

int PhotoView2::getScrollHeight() const
{
    Gtk::Viewport* parent = getViewport();

    Gtk::Allocation allocation = parent->get_allocation();
    return allocation.get_height();
}

void PhotoView2::moveCursor(vector<PhotoIcon*>::iterator pos)
{
    if (m_photoCursor != m_photos.end())
    {
        (*m_photoCursor)->selected = false;
    }
    m_photoCursor = pos;
    updateCursor();
}

void PhotoView2::moveCursor(int a)
{
    if (m_photoCursor == m_photos.end())
    {
        return;
    }

    (*m_photoCursor)->selected = false;

    int i;
    if (a > 0)
    {
        for (
            i = 1;
            i <= a && (m_photoCursor + 1) != m_photos.end();
            i++)
        {
            m_photoCursor++;
        }
    }
    else
    {
        for (
            i = 1;
            i <= -a && (m_photoCursor) != m_photos.begin();
            i++)
        {
            m_photoCursor--;
        }
    }

    updateCursor();
}

void PhotoView2::movePage(int a)
{
    if (m_photoCursor == m_photos.end())
    {
        return;
    }

    (*m_photoCursor)->selected = false;

    Glib::RefPtr<Gtk::Adjustment> adj = getScrollAdjustment();
    int y = adj->get_value();
    int height = getScrollHeight();
    int currentX = (*m_photoCursor)->x;
    int currentY = (*m_photoCursor)->y;
    int currentHeight = (*m_photoCursor)->height;

    if (a > 0)
    {
        for ( ; (m_photoCursor + 1) != m_photos.end(); m_photoCursor++)
        {
            PhotoIcon* icon = *m_photoCursor;
            if (icon->y + icon->height >= currentY + currentHeight + height &&
                icon->x >= currentX)
            {
                break;
            }
        }
    }
    else
    {
        for ( ; (m_photoCursor) != m_photos.begin(); m_photoCursor--)
        {
            PhotoIcon* icon = *m_photoCursor;
            if (icon->y + icon->height < currentY - height &&
                icon->x <= currentX)
            {
                break;
            }
        }
    }

    updateCursor();
}

void PhotoView2::updateCursor()
{
    (*m_photoCursor)->selected = true;
    m_library->displayDetails((*m_photoCursor)->photo);
    scrollToCursor();
    queue_draw();
}

void PhotoView2::scrollToCursor()
{
    if (m_photoCursor == m_photos.end())
    {
        // No where to scroll to!
        return;
    }
    PhotoIcon* icon = *m_photoCursor;

    Glib::RefPtr<Gtk::Adjustment> adj = getScrollAdjustment();
    int y = adj->get_value();

    int height = getScrollHeight();

    printf("PhotoView2::scrollToCursor: height=%d\n", height);

    if (y + height < icon->y + icon->height)
    {
        adj->set_value((icon->y + icon->height) - height);
    }
    else if (y > icon->y)
    {
        adj->set_value(icon->y);
    }
}

