
#include "editpreview.h"

#include <cairomm/xlib_surface.h>

using namespace Geek::Gfx;

EditPreview::EditPreview()
{
    m_workflow = NULL;
    m_surface = NULL;

    override_background_color(Gdk::RGBA("#101010"));
}

EditPreview::~EditPreview()
{
}

void EditPreview::setWorkflow(Workflow* workflow)
{
    m_workflow = workflow;
    render();
}

void EditPreview::render()
{
    m_surface = Surface::loadJPEG(m_workflow->getFile()->getPath());

    queue_draw();
}

bool EditPreview::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    if (m_workflow == NULL || m_surface == NULL)
    {
        return true;
    }

    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    Cairo::RefPtr<Cairo::Surface> surface = cr->get_group_target();

    Surface* resized = m_surface->scaleToFit(width, height, false);

    Cairo::RefPtr<Cairo::ImageSurface> cairoSurface;
    cairoSurface = Cairo::ImageSurface::create(
        resized->getData(),
        Cairo::FORMAT_RGB24,
        resized->getWidth(),
        resized->getHeight(),
        Cairo::ImageSurface::format_stride_for_width(
            Cairo::FORMAT_RGB24,
            resized->getWidth()));

    // Center the preview
    float x = ((float)width / 2.0f) - (resized->getWidth() / 2.0f);
    float y = ((float)height / 2.0f) - (resized->getHeight() / 2.0f);

    cr->set_source(cairoSurface, x, y);

    cr->paint();

    delete resized;

    return true;
}

