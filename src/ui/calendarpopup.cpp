
#include <fotofing/utils.h>

#include "calendarpopup.h"

CalendarPopup::CalendarPopup(Gtk::Window& parent, time_t time) :
    Gtk::Dialog("Date", parent, true),
    m_hourAdjustment(Gtk::Adjustment::create(0.0f, 0.0f, 23.0f)),
    m_minuteAdjustment(Gtk::Adjustment::create(0.0f, 0.0f, 59.0f)),
    m_hour(m_hourAdjustment),
    m_minute(m_minuteAdjustment),
    m_timeSeparator(":")
{
    m_time = time;

    m_box.pack_start(m_calendar, Gtk::PACK_EXPAND_WIDGET);
    m_timeBox.pack_start(m_hour, Gtk::PACK_EXPAND_WIDGET);
    m_timeBox.pack_start(m_timeSeparator, Gtk::PACK_SHRINK);
    m_timeBox.pack_start(m_minute, Gtk::PACK_EXPAND_WIDGET);
    m_box.pack_start(m_timeBox, Gtk::PACK_SHRINK);
    get_content_area()->pack_start(m_box, Gtk::PACK_SHRINK);

    struct tm tm;
    localtime_r(&m_time, &tm);
    m_calendar.select_month(tm.tm_mon, tm.tm_year + 1900);
    m_calendar.select_day(tm.tm_mday);

    m_calendar.signal_day_selected_double_click().connect(sigc::mem_fun(
        *this,
        &CalendarPopup::onDaySelected));

    show_all();
}

CalendarPopup::~CalendarPopup()
{
}

void CalendarPopup::onDaySelected()
{
    Glib::Date date;
    m_calendar.get_date(date);

    struct tm tm;
    date.to_struct_tm(tm);

    tm.tm_hour = m_hour.get_value_as_int();
    tm.tm_min = m_minute.get_value_as_int();
    tm.tm_sec = 0;

    m_time = tm2time(&tm);
    printf("CalendarPopup::onDaySelected: m_time=%ld\n", m_time);

    response(Gtk::RESPONSE_OK);
}

