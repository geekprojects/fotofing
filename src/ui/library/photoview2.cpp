
#include "library/photoview2.h"
#include "mainwindow.h"
#include "uiutils.h"

#include <algorithm>

#include <fotofing/index.h>

using namespace std;
using namespace Geek::Gfx;

PhotoView2::PhotoView2(Index* index)
{
    m_index = index;
    override_background_color(Gdk::RGBA("#101010"));

    m_padding = 20;
    m_margin = 0;
    m_titleHeight = 20;

    m_sortBy = PHOTOVIEW_SORT_TIMESTAMP;
    m_sortDir = true;

    m_photoCursor = m_photos.end();

    set_can_focus(true);
    set_vexpand(true);
    set_vscroll_policy(Gtk::SCROLL_NATURAL);

    set_events(Gdk::BUTTON_PRESS_MASK | Gdk::KEY_PRESS_MASK);

    // Add context menu
    Gtk::MenuItem* item = Gtk::manage(new Gtk::MenuItem("_Add Tag", true));
/*
    item->signal_activate().connect(sigc::mem_fun(
        *this,
        &PhotoView::addTag));
*/
    m_popupMenu.append(*item);
    item = Gtk::manage(new Gtk::MenuItem("_Rename", true));
    item->signal_activate().connect(sigc::mem_fun(
        *this,
        &PhotoView2::rename));
    m_popupMenu.append(*item);
    //m_popupMenu.accelerate(this);
    m_popupMenu.show_all();

    signal_popup_menu().connect(sigc::mem_fun(
        *this,
        &PhotoView2::onPopupMenu));
}

PhotoView2::~PhotoView2()
{
    clearPhotos();
}

static bool sortPhotoIcons(PhotoIcon* l, PhotoIcon* r)
{
    switch (l->photoView->getSortBy())
    {
        case PHOTOVIEW_SORT_TITLE:
        {
            bool lHasTitle = l->title != l->photo->getId().substr(0, 6) + "...";
            bool rHasTitle = r->title != r->photo->getId().substr(0, 6) + "...";
            if (lHasTitle != rHasTitle)
            {
                return lHasTitle;
            }
            else if (l->photoView->getSortDir())
            {
                return l->title < r->title;
            }
            else
            {
                return l->title > r->title;
            }
        } break;

        case PHOTOVIEW_SORT_TIMESTAMP:
        default:
            if (l->photoView->getSortDir())
            {
                return l->photo->getTimestamp() < r->photo->getTimestamp();
            }
            else
            {
                return l->photo->getTimestamp() > r->photo->getTimestamp();
            }
            break;
    }
}

void PhotoView2::update(vector<Photo*> photos)
{
    clearPhotos();

    m_maxThumbWidth = 0;

    vector<Photo*>::iterator it;
    for (it = photos.begin(); it != photos.end(); it++)
    {
        PhotoIcon* icon = new PhotoIcon();
        icon->photoView = this;
        icon->photo = *it;
        icon->selected = false;

        string title = m_index->getProperty(icon->photo->getId(), "Title");
        if (title == "")
        {
            title = icon->photo->getId().substr(0, 6) + "...";
        }
        icon->title = title;

        Surface* thumb = icon->photo->getThumbnail();

        // Rotate thumbnail if necessary
        TagData* orientation = m_index->getTagData(
            icon->photo->getId(),
            "Photo/Orientation");
        if (orientation->type == SQLITE_INTEGER)
        {
            switch (orientation->data.i)
            {
                case 6:
                    thumb->rotate(90);
                    break;
                case 8:
                    thumb->rotate(270);
                    break;
            }
        }
        delete orientation;

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
    m_photoCursor = m_photos.end();

    sort();

    queue_resize();
}

void PhotoView2::setSort(PhotoViewSort sortBy, bool direction)
{
    if (sortBy == m_sortBy && direction == m_sortDir)
    {
        // No point!
        return;
    }

    m_sortBy = sortBy;
    m_sortDir = direction;

    if (m_photos.size() > 0)
    {
        sort();

        // All of the icons positions will require recalculating
        queue_resize();
    }
}

void PhotoView2::sort()
{
    // This will invalidate our cursor!
    PhotoIcon* cursor = NULL;
    if (!(m_photoCursor == m_photos.end()))
    {
        cursor = (*m_photoCursor);
    }
    std::sort(m_photos.begin(), m_photos.end(), sortPhotoIcons);

    if (cursor != NULL)
    {
        moveCursor(cursor);
    }
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
    double clipX1;
    double clipY1;
    double clipX2;
    double clipY2;

    cr->get_clip_extents(clipX1, clipY1, clipX2, clipY2);
#if 0
    printf("PhotoView2::on_draw: clipY1=%0.2f\n", clipY1);
#endif

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

        cr->set_line_width(2.0);

        // Create the linear gradient diagonal
        Cairo::RefPtr<Cairo::LinearGradient> gradient;
        gradient = Cairo::LinearGradient::create(
            icon->x,
            icon->y + 20,
            icon->x,
            icon->y + icon->height);
        if (icon->selected)
        {
            gradient->add_color_stop_rgb(0, 0.5, 0.5, 0.5);
            gradient->add_color_stop_rgb(1, 0.3, 0.3, 0.3);
        }
        else
        {
            gradient->add_color_stop_rgb(0, 0.1, 0.1, 0.1);
            gradient->add_color_stop_rgb(1, 0.06, 0.06, 0.06);
        }


        cr->rectangle(
            icon->x,
            icon->y,
            icon->width - 1,
            icon->height - 1);

        cr->set_source (gradient);
        cr->fill_preserve();

        Cairo::RefPtr<Cairo::LinearGradient> borderGradient;
        borderGradient = Cairo::LinearGradient::create(
            icon->x,
            icon->y,
            icon->x,
            icon->y + icon->height);

        if (it == m_photoCursor)
        {
            borderGradient->add_color_stop_rgb(0, 0.6, 0.6, 0.6);
            borderGradient->add_color_stop_rgb(1, 0.3, 0.3, 0.3);
        }
        else if (icon->selected)
        {

            borderGradient->add_color_stop_rgb(0, 0.3, 0.3, 0.3);
            borderGradient->add_color_stop_rgb(1, 0.1, 0.1, 0.1);
        }
        else
        {
            borderGradient->add_color_stop_rgb(0, 0.06, 0.06, 0.06);
            borderGradient->add_color_stop_rgb(1, 0.0, 0.0, 0.0);
        }

        cr->set_line_width(2.0);
        cr->set_source(borderGradient);
        cr->stroke();

        cr->set_source_rgb(0.02, 0.02, 0.02);

        // Border around the photo itself
        cr->rectangle(
            photoX + 3,
            photoY + 3,
            thumb->getWidth() - 2,
            thumb->getHeight() - 2);
        cr->fill_preserve();
        cr->stroke();

        Glib::RefPtr<Pango::Layout> layout = create_pango_layout(icon->title);

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
#if 0
    printf("PhotoView2::on_draw: Skipped=%d\n", skipped);
#endif

    return true;
}

bool PhotoView2::on_button_press_event(GdkEventButton* event)
{
#if 0
    printf("PhotoView2::on_button_press_event: x=%0.2f, y=%0.2f\n", event->x, event->y);
#endif

    vector<PhotoIcon*>::iterator prevCursor = m_photoCursor;

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
        if (event->type == GDK_BUTTON_PRESS)
        {
            if (!(event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)))
            {
                clearSelection();
            }
            else if (event->state & GDK_CONTROL_MASK &&
                prevCursor != m_photos.end())
            {
                vector<PhotoIcon*>::iterator start;
                vector<PhotoIcon*>::iterator end;
                if (prevCursor < m_photoCursor)
                {
                    start = prevCursor;
                    end = m_photoCursor;
                }
                else
                {
                    start = m_photoCursor;
                    end = prevCursor;
                }
                for (it = start; it != end; it++)
                {
                    (*it)->selected = true;
                }
            }

            moveCursor(selected);

            if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
            {
                printf("PhotoView2::on_button_press_event: Context menu!\n");
                m_popupMenu.popup(event->button, event->time);
            }
        }
        else if (event->type == GDK_2BUTTON_PRESS)
        {
            printf("PhotoView2::on_button_press_event: Double click!\n");
            m_activatePhotoSignal.emit(selected->photo);
        }

   }

    return true;
}

bool PhotoView2::on_key_press_event(GdkEventKey* event)
{
    switch (event->keyval)
    {
        case GDK_KEY_Left:
        case GDK_KEY_Right:
        case GDK_KEY_Up:
        case GDK_KEY_Home:
        case GDK_KEY_End:
        case GDK_KEY_Down:
        case GDK_KEY_Page_Up:
        case GDK_KEY_Page_Down:
            if (!(event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)))
            {
                clearSelection();
            }
    }

    bool selectAll = (event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK));

    switch (event->keyval)
    {
        case GDK_KEY_Left:
            moveCursor(-1, selectAll);
            break;

        case GDK_KEY_Right:
            moveCursor(1, selectAll);
            break;

        case GDK_KEY_Up:
            moveCursor(-m_columns, selectAll);
            break;

        case GDK_KEY_Home:
            moveCursor(m_photos.begin());
            break;

        case GDK_KEY_End:
            moveCursor(m_photos.end() - 1);
            break;

        case GDK_KEY_Down:
            moveCursor(m_columns, selectAll);
            break;

        case GDK_KEY_Page_Up:
            movePage(-1, selectAll);
            break;

        case GDK_KEY_Page_Down:
            movePage(1, selectAll);
            break;

        case GDK_KEY_Return:
            m_activatePhotoSignal.emit((*m_photoCursor)->photo);
            break;
        default:
            printf(
                "PhotoView2::on_key_press_event: Unhandled key=0x%x\n",
                event->keyval);
    }
    return true;
}

vector<Photo*> PhotoView2::getSelectedPhotos()
{
    vector<Photo*> results;

    vector<PhotoIcon*>::iterator it;
    for (it = m_photos.begin(); it != m_photos.end(); it++)
    {
        PhotoIcon* icon = *it;
        if (icon->selected)
        {
            results.push_back(icon->photo);
        }
    }

    return results;
}

void PhotoView2::rename()
{
    if (m_photoCursor == m_photos.end())
    {
        return;
    }

    Photo* photo = (*m_photoCursor)->photo;

    string title = m_index->getProperty(photo->getId(), "Title");

    bool res;
    res = UIUtils::promptString(
        *((Gtk::Window*)get_toplevel()),
        "Photo Title",
        "Please give this photo a title",
        title,
        title);
    if (res)
    {
        m_index->setProperty(photo->getId(), "Title", title);
        m_cursorChangedSignal.emit(photo);

        (*m_photoCursor)->title = title;
        queue_draw();
    }
}

bool PhotoView2::onPopupMenu()
{
printf("PhotoView2::onPopupMenu: Called!\n");
    return true;
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

void PhotoView2::moveCursor(PhotoIcon* icon)
{
    vector<PhotoIcon*>::iterator it;
    for (it = m_photos.begin(); it != m_photos.end(); it++)
    {
        if (*it == icon)
        {
            moveCursor(it);
            return;
        }
    }
}

void PhotoView2::moveCursor(vector<PhotoIcon*>::iterator pos)
{
    m_photoCursor = pos;
    updateCursor();
}

void PhotoView2::moveCursor(int a, bool select)
{
    if (m_photoCursor == m_photos.end())
    {
        return;
    }

    int i;
    if (a > 0)
    {
        for (
            i = 1;
            i <= a && (m_photoCursor + 1) != m_photos.end();
            i++, m_photoCursor++)
        {
            if (select)
            {
                (*m_photoCursor)->selected = true;
            }
        }
    }
    else
    {
        for (
            i = 1;
            i <= -a && (m_photoCursor) != m_photos.begin();
            i++, m_photoCursor--)
        {
            if (select)
            {
                (*m_photoCursor)->selected = true;
            }
        }
    }

    updateCursor();
}

void PhotoView2::movePage(int a, bool selectPage)
{
    if (m_photoCursor == m_photos.end())
    {
        return;
    }

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

            if (selectPage)
            {
                icon->selected = true;
            }

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

            if (selectPage)
            {
                icon->selected = true;
            }

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
    m_cursorChangedSignal.emit((*m_photoCursor)->photo);
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
    scrollToIcon(*m_photoCursor);
}

void PhotoView2::scrollToIcon(PhotoIcon* icon)
{
    Glib::RefPtr<Gtk::Adjustment> adj = getScrollAdjustment();
    int y = adj->get_value();

    int height = getScrollHeight();

#if 0
    printf("PhotoView2::scrollToCursor: height=%d\n", height);
#endif

    if (y + height < icon->y + icon->height)
    {
        adj->set_value((icon->y + icon->height) - height);
    }
    else if (y > icon->y)
    {
        adj->set_value(icon->y);
    }
}

sigc::signal<void, Photo*>& PhotoView2::signal_cursor_changed()
{
    return m_cursorChangedSignal;
}

sigc::signal<void, Photo*>& PhotoView2::signal_activate_photo()
{
    return m_activatePhotoSignal;
}

