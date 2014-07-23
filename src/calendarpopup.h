#ifndef __FOTOFING_CALEANDARPOPUP_H_
#define __FOTOFING_CALEANDARPOPUP_H_

#include <gtkmm.h>

class CalendarPopup : public Gtk::Dialog
{
 private:
    Gtk::VBox m_box;
    Gtk::HBox m_timeBox;

    Gtk::Calendar m_calendar;

    Glib::RefPtr<Gtk::Adjustment> m_hourAdjustment;
    Glib::RefPtr<Gtk::Adjustment> m_minuteAdjustment;
    Gtk::SpinButton m_hour;
    Gtk::SpinButton m_minute;
    Gtk::Label m_timeSeparator;

    time_t m_time;

 public:
    CalendarPopup(Gtk::Window& parent, time_t time);
    ~CalendarPopup();

    void onDaySelected();

    time_t getTime() { return m_time; }
};

#endif
