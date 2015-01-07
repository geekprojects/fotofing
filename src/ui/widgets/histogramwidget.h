#ifndef __FOTOFING_UI_HISTOGRAM_WIDGET_H_
#define __FOTOFING_UI_HISTOGRAM_WIDGET_H_

#include <gtkmm.h>

class HistogramWidget : public ::Gtk::DrawingArea
{
 private:
    bool m_valid;
    float m_histogram[3][256];

 public:
    HistogramWidget();
    ~HistogramWidget();

    void setHistogram(float* red, float* green, float* blue);
    void clearHistogram();

    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
};

#endif
