
#include <fotofing/utils.h>

#include "mainwindow.h"
#include "calendarpopup.h"
#include "sourcesdialog.h"
#include "tagdialog.h"
#include "uiutils.h"

using namespace std;

MainWindow::MainWindow(Index* index) :
    m_library(this),
    m_edit("Edit tab")
{
    m_index = index;

    set_title("Fotofing");
    set_default_size(600, 400);

    m_tabs.set_name("ff-content");
    m_tabs.append_page(m_library, "Library");
    m_tabs.append_page(m_edit, "Edit");

    m_progressBar.set_show_text(true);
    m_progressBar.set_ellipsize(Pango::ELLIPSIZE_START);
    m_statusBox.pack_start(m_statusBar, Gtk::PACK_EXPAND_WIDGET);
    m_statusBox.pack_start(m_progressBar, Gtk::PACK_SHRINK, 5);

    Gtk::MenuBar* menuBar = createMenu();
    m_vBox.pack_start(*menuBar, Gtk::PACK_SHRINK);
    m_vBox.pack_start(m_tabs, Gtk::PACK_EXPAND_WIDGET);
    m_vBox.pack_start(m_statusBox, Gtk::PACK_SHRINK);
    add(m_vBox);

    show_all();

    Glib::RefPtr<Gtk::StyleContext> styleContext = get_style_context();

    Glib::RefPtr<Gtk::CssProvider> refStyleProvider = Gtk::CssProvider::create();

    refStyleProvider->load_from_data(
        "#ff-content GtkIconView { background-color: #080808; }"
        "#ff-content GtkIconView { border: 1px solid #ffffff }"
        "#ff-content GtkIconView.view.cell { color: #ffffff; text-shadow: 1 1 black; }"
        "#ff-content GtkIconView.view.cell:selected { background-color: #101010; background-image: none; text-shadow: 1 1 black; }"
    );

    Gtk::StyleContext::add_provider_for_screen(get_screen(), refStyleProvider, 800);

    m_library.updateSources();
}

MainWindow::~MainWindow()
{
}

Gtk::MenuBar* MainWindow::createMenu()
{
    Glib::RefPtr<Gio::SimpleActionGroup> actionGroup;
    actionGroup = Gio::SimpleActionGroup::create();

    actionGroup->add_action(
        "about",
        sigc::mem_fun(m_about, &About::open));

    actionGroup->add_action(
        "sources",
        sigc::mem_fun(m_library, &Library::openSourcesDialog));

/*
    actionGroup->add_action(
        "selectAll",
        sigc::mem_fun(m_photoView, &PhotoView::selectAll));
*/

    insert_action_group("fotofing", actionGroup);

    m_refBuilder = Gtk::Builder::create();

    Glib::ustring ui_info =
        "<interface>"
        "  <menu id='main-menu'>"
        "    <submenu>"
        "      <attribute name='label' translatable='yes'>_File</attribute>"
        "        <section>"
        "          <item>"
        "            <attribute name='label' translatable='yes'>_Import Index</attribute>"
        "            <attribute name='action' translatable='yes'>fotofing.importindex</attribute>"
        "          </item>"
        "        </section>"
        "        <section>"
        "          <item>"
        "            <attribute name='label' translatable='yes'>_Sources...</attribute>"
        "            <attribute name='action' translatable='yes'>fotofing.sources</attribute>"
        "          </item>"
        "        </section>"
        "    </submenu>"
        "    <submenu>"
        "      <attribute name='label' translatable='yes'>_Edit</attribute>"
        "        <section>"
        "          <item>"
        "            <attribute name='label' translatable='yes'>Select _All</attribute>"
        "            <attribute name='action' translatable='yes'>fotofing.selectAll</attribute>"
        "            <attribute name='accel'>&lt;Primary&gt;a</attribute>"
        "          </item>"
        "        </section>"
        "        <section>"
        "          <item>"
        "            <attribute name='label' translatable='yes'>Preferences</attribute>"
        "            <attribute name='action' translatable='yes'>fotofing.preferences</attribute>"
        "          </item>"
        "        </section>"
        "    </submenu>"
        "    <submenu>"
        "      <attribute name='label' translatable='yes'>_Help</attribute>"
        "        <section>"
        "          <item>"
        "            <attribute name='label' translatable='yes'>About</attribute>"
        "            <attribute name='action' translatable='yes'>fotofing.about</attribute>"
        "          </item>"
        "        </section>"
        "    </submenu>"
        "  </menu>"
        "</interface>";

    m_refBuilder->add_from_string(ui_info);

    Glib::RefPtr<Glib::Object> object = m_refBuilder->get_object("main-menu");
    Glib::RefPtr<Gio::Menu> gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);

    return Gtk::manage(new Gtk::MenuBar(gmenu));
}

void MainWindow::startProgress()
{
    m_progressActive = true;
    m_progressBar.pulse();
    Glib::signal_timeout().connect(sigc::mem_fun(
        *this,
        &MainWindow::progressTimeout), 50 );
}

void MainWindow::endProgress()
{
    m_progressActive = false;
}

void MainWindow::updateProgress(
    int complete,
    int total,
    std::string info)
{
    m_progressBar.set_fraction((float)complete / (float)total);
    m_progressBar.set_text(info);
}

bool MainWindow::progressTimeout()
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

