
#include <fotofing/utils.h>

#include "window.h"
#include "uiutils.h"

using namespace std;

FotofingWindow::FotofingWindow()
{
    set_title("Fotofing");
    set_default_size(600, 400);

    m_tabs.set_name("ff-content");
    m_tabs.signal_switch_page().connect(sigc::mem_fun(
        *this,
        &FotofingWindow::onTabSwitch));

    m_statusBar.set_justify(Gtk::JUSTIFY_LEFT);
    m_statusBar.set_alignment(Gtk::ALIGN_START);
    m_progressBar.set_show_text(true);
    m_progressBar.set_ellipsize(Pango::ELLIPSIZE_START);
    m_statusBox.pack_start(m_statusBar, Gtk::PACK_EXPAND_WIDGET);
    m_statusBox.pack_start(m_progressBar, Gtk::PACK_SHRINK, 5);

    m_vBox.pack_start(m_menuBar, Gtk::PACK_SHRINK);
    m_vBox.pack_start(m_tabs, Gtk::PACK_EXPAND_WIDGET);
    m_vBox.pack_start(m_statusBox, Gtk::PACK_SHRINK);
    add(m_vBox);

    show_all();

    Glib::RefPtr<Gtk::StyleContext> styleContext = get_style_context();

    Glib::RefPtr<Gtk::CssProvider> refStyleProvider = Gtk::CssProvider::create();

    refStyleProvider->load_from_data(
        "#ff-content GtkIconView { background-color: #080808; }"
        "#ff-content GtkIconView { border: 1px solid #ffffff }"
        "#ff-content GtkIconView.view.cell { color: #ffffff; text-shadow: 1px 1px black; }"
        "#ff-content GtkIconView.view.cell:selected { background-color: #101010; background-image: none; text-shadow: 1px 1px black; }"
    );

    Gtk::StyleContext::add_provider_for_screen(get_screen(), refStyleProvider, 800);
}

FotofingWindow::~FotofingWindow()
{
}

void FotofingWindow::openTab(Tab* w)
{
    int page = m_tabs.append_page(*w, *(w->getTabLabel()));
    m_tabs.set_current_page(page);
}

void FotofingWindow::closeTab(Tab* w)
{
    m_tabs.remove_page(*w);
}

bool FotofingWindow::isTabVisible(Gtk::Widget* w)
{
    return (m_tabs.page_num(*w) == m_tabs.get_current_page());
}

void FotofingWindow::onTabSwitch(Gtk::Widget* w, guint pageNum)
{
    Tab* tab = (Tab*)w;
    printf("FotofingWindow::onTabSwitch: pageNum=%d\n", pageNum);
    m_menuBar.bind_model(tab->getMenu(), true);

    if (m_tabs.get_n_pages() == 1)
    {
        m_tabs.set_show_tabs(false);
    }
    else
    {
        m_tabs.set_show_tabs(true);
    }
}

Tab* FotofingWindow::getCurrentTab()
{
    return (Tab*)m_tabs.get_nth_page(m_tabs.get_current_page());
}

void FotofingWindow::setStatusMessage(string message)
{
    m_statusBar.set_label(message);
}

void FotofingWindow::startProgress()
{
    m_progressActive = true;
    m_progressBar.pulse();
    Glib::signal_timeout().connect(sigc::mem_fun(
        *this,
        &FotofingWindow::progressTimeout), 50 );
}

void FotofingWindow::endProgress()
{
    m_progressActive = false;
}

void FotofingWindow::updateProgress(
    int complete,
    int total,
    std::string info)
{
    m_progressBar.set_fraction((float)complete / (float)total);
    m_progressBar.set_text(info);
}

bool FotofingWindow::progressTimeout()
{
    bool active = m_progressActive;
    if (active)
    {
        m_progressBar.pulse();
    }
    else
    {
        m_progressBar.set_fraction(0.0f);
    }
    return active;
}

