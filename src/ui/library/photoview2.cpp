
#include "library/photoview2.h"
#include "library/library.h"

using namespace std;
using namespace Geek::Gfx;

PhotoView2::PhotoView2(Library* library)
{
    m_library = library;
    override_background_color(Gdk::RGBA("#101010"));

    m_padding = 20;
    m_margin = 0;
    m_titleHeight = 20;

    set_vexpand(true);
    set_vscroll_policy(Gtk::SCROLL_NATURAL);
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
        icon->selected = (it == photos.begin());

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

    int maxCols = width / iconWidth;

    int rows = width / maxCols;
    if (width % maxCols > 0)
    {
        rows++;
    }

    int row = 0;

    printf(
        "PhotoView2::on_size_allocate: width=%d, height=%d\n",
        width,
        height);
    printf(
        "PhotoView2::on_size_allocate: maxCols=%d (%d / %d)\n",
        maxCols,
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
        for (col = 0; it != m_photos.end() && col < maxCols; it++, col++)
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
}


