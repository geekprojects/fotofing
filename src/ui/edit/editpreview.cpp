
#include "editpreview.h"
#include "edit.h"

#include <cairomm/xlib_surface.h>

using namespace std;
using namespace Geek::Gfx;

EditPreview::EditPreview(Edit* edit)
{
    m_edit = edit;
    m_workflow = NULL;
    m_original = NULL;
    m_rendered = NULL;

    override_background_color(Gdk::RGBA("#101010"));
}

EditPreview::~EditPreview()
{
}

void EditPreview::setWorkflow(Workflow* workflow)
{
    m_workflow = workflow;

    m_original = Surface::loadJPEG(m_workflow->getFile()->getPath());

    render(true);
}

void EditPreview::render(bool opsChanged)
{
    if (m_original == NULL)
    {
        return;
    }

    if (!m_edit->isVisible())
    {
        printf("EditPreview::render: Not visible, no point\n");
        return;
    }

    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    float zx = (float)width / (float)m_original->getWidth();
    float zy = (float)height / (float)m_original->getHeight();
    float scale = MIN(zx, zy);

    if (!opsChanged &&
        m_rendered != NULL &&
        m_rendered->getWidth() == (int)((float)m_original->getWidth() * scale))
    {
        printf("EditPreview::render: Skipping render\n");
        return;
    }

    if (m_rendered != NULL)
    {
        delete m_rendered;
    }

    m_rendered = m_original->scale(scale, false);

    vector<OperationInstance*>::iterator it;
    for (
        it = m_workflow->getOperations().begin();
        it != m_workflow->getOperations().end();
        it++)
    {
        OperationInstance* op = *it;
        printf("EditPreview::render:  -> applying %s\n", op->getOperation()->getName().c_str());
        op->apply(m_rendered, NULL);
        printf("EditPreview::render:  -> finished applying %s\n", op->getOperation()->getName().c_str());
    }

    queue_draw();
}

void EditPreview::on_size_allocate(Gtk::Allocation& allocation)
{
    Gtk::DrawingArea::on_size_allocate(allocation);

    render(false);
}

bool EditPreview::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    if (m_workflow == NULL || m_rendered == NULL)
    {
        return true;
    }

    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    Cairo::RefPtr<Cairo::Surface> surface = cr->get_group_target();

    Cairo::RefPtr<Cairo::ImageSurface> cairoSurface;
    cairoSurface = Cairo::ImageSurface::create(
        m_rendered->getData(),
        Cairo::FORMAT_RGB24,
        m_rendered->getWidth(),
        m_rendered->getHeight(),
        Cairo::ImageSurface::format_stride_for_width(
            Cairo::FORMAT_RGB24,
            m_rendered->getWidth()));

    // Center the preview
    float x = ((float)width / 2.0f) - (m_rendered->getWidth() / 2.0f);
    float y = ((float)height / 2.0f) - (m_rendered->getHeight() / 2.0f);

    cr->set_source(cairoSurface, x, y);

    cr->paint();

    return true;
}

