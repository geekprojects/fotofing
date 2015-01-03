
#include "histogramwidget.h"

HistogramWidget::HistogramWidget()
{
    m_valid = false;
    set_size_request(256, 100);
}

HistogramWidget::~HistogramWidget()
{
}


void HistogramWidget::setHistogram(float* red, float* green, float* blue)
{
    memcpy(m_histogram[0], red, sizeof(float) * 256);
    memcpy(m_histogram[1], green, sizeof(float) * 256);
    memcpy(m_histogram[2], blue, sizeof(float) * 256);
    m_valid = true;

    queue_draw();
}

void HistogramWidget::clearHistogram()
{
    m_valid = false;

    queue_draw();
}

bool HistogramWidget::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    override_background_color(Gdk::RGBA("#101010"));

    if (!m_valid)
    {
        return true;
    }
    ::Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    int channel;
    for (channel = 0; channel < 3; channel++)
    {
        int i;
        switch (channel)
        {
            case 0:
                cr->set_source_rgba(1.0, 0.0, 0.0, 0.8);
                break;
            case 1:
                cr->set_source_rgba(0.0, 1.0, 0.0, 0.8);
                break;
            case 2:
                cr->set_source_rgba(0.0, 0.0, 1.0, 0.8);
                break;
        }
        for (i = 0; i < 256; i++)
        {
            int ry = height - ((float)height * m_histogram[channel][i]);
#if 0
            if (i == 0)
            {
                cr->move_to(i, ry);
            }
            else
            {
                cr->line_to(i, ry);
            }
#endif

            cr->move_to(i, height);
            cr->line_to(i, ry);
        }
        cr->stroke();
    }
    return true;
}



