
#include <fotofing/utils.h>

#include "mainwindow.h"
#include "calendarpopup.h"
#include "sourcesdialog.h"

using namespace std;

MainWindow::MainWindow(Index* index) :
    m_dateSeparator("-"),
    m_tagSearchButton("Search"),
    m_photoView(this),
    m_photoDetails(this)
{
    m_index = index;
    m_progressActive = false;

    set_title("Fotofing");
    set_default_size(600, 400);

    // Toolbar
    m_toolbarTag.set_icon_name("tag-new");
    m_toolbarTag.set_tooltip_text("Add Tag");
    m_toolbar.append(m_toolbarTag);

    m_toolbarTagEvent.set_icon_name("appointment-new");
    m_toolbarTagEvent.set_tooltip_text("Add Event Tag");
    m_toolbar.append(m_toolbarTagEvent);

    m_toolbar.append(*(Gtk::manage(new Gtk::SeparatorToolItem)));

    m_toolbarHide.set_icon_name("edit-delete");
    m_toolbarHide.set_tooltip_text("Hide");
    m_toolbar.append(m_toolbarHide);

    m_fromDate = 0;
    m_toDate = time(NULL);
    m_fromDateButton.signal_clicked().connect(sigc::mem_fun(
        *this,
        &MainWindow::onFromDateClicked));
    m_toDateButton.signal_clicked().connect(sigc::mem_fun(
        *this,
        &MainWindow::onToDateClicked));
    updateDateButtons();

    m_toolbarBox.pack_start(m_toolbar, Gtk::PACK_EXPAND_WIDGET);
    m_toolbarBox.pack_start(m_fromDateButton, Gtk::PACK_SHRINK);
    m_toolbarBox.pack_start(m_dateSeparator, Gtk::PACK_SHRINK);
    m_toolbarBox.pack_start(m_toDateButton, Gtk::PACK_SHRINK);


    m_tagSearchButton.set_tooltip_text("Search for selected tags");
    m_tagSearchButton.signal_clicked().connect(sigc::mem_fun(
        *this,
        &MainWindow::onTagSearchClicked));

    // Tag Tree: Add it
    m_allTagsView.signal_row_activate().connect(sigc::mem_fun(
        *this,
        &MainWindow::update));
    m_tagBox.pack_start(m_allTagsView, Gtk::PACK_EXPAND_WIDGET);
    m_tagBox.pack_start(m_tagSearchButton, Gtk::PACK_SHRINK);
    m_tagFrame.add(m_tagBox);

    m_hBox.pack_start(m_tagFrame, Gtk::PACK_SHRINK);
    m_hBox.pack_start(m_photoView, Gtk::PACK_EXPAND_WIDGET);
    m_hBox.pack_start(m_photoDetails, Gtk::PACK_SHRINK);

    m_statusBox.pack_start(m_statusBar, Gtk::PACK_EXPAND_WIDGET);
    m_statusBox.pack_start(m_progressBar, Gtk::PACK_SHRINK, 5);

    Gtk::MenuBar* menuBar = createMenu();
    m_vBox.pack_start(*menuBar, Gtk::PACK_SHRINK);
    m_vBox.pack_start(m_toolbarBox, Gtk::PACK_SHRINK);
    m_vBox.pack_start(m_hBox, Gtk::PACK_EXPAND_WIDGET);
    m_vBox.pack_start(m_statusBox, Gtk::PACK_SHRINK);
    add(m_vBox);

    show_all();

    updateTags();
    update();
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
        sigc::mem_fun(*this, &MainWindow::openSourcesDialog));

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

void MainWindow::update()
{
    m_progressActive = true;
    m_progressBar.pulse();
    Glib::signal_timeout().connect(sigc::mem_fun(
        *this,
        &MainWindow::progressTimeout), 50 );

    vector<Tag*> tags = m_allTagsView.getSelectedTags();
    m_photoView.update(tags, m_fromDate, m_toDate);

    //m_progressBar.set_fraction(0.75f);
    updateDateButtons();

    //m_progressBar.set_fraction(1.0f);
    m_progressActive = false;
}

void MainWindow::displayDetails(Photo* photo)
{
    m_photoDetails.displayDetails(photo);
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

void MainWindow::onFromDateClicked()
{
    CalendarPopup* cp = new CalendarPopup(*this, m_fromDate);
    cp->run();

    m_fromDate = cp->getTime();
    delete cp;

    updateDateButtons();
    update();
}

void MainWindow::onToDateClicked()
{
    CalendarPopup* cp = new CalendarPopup(*this, m_toDate);
    cp->run();

    m_toDate = cp->getTime();
    delete cp;

    updateDateButtons();
    update();
}

void MainWindow::updateDateButtons()
{
    if (m_fromDate > 0)
    {
        m_fromDateButton.set_label(timeToString(m_fromDate));
    }
    else
    {
        m_fromDateButton.set_label("-");
    }

    if (m_toDate > 0)
    {
        m_toDateButton.set_label(timeToString(m_toDate));
    }
    else
    {
        m_toDateButton.set_label("-");
    }
}

void MainWindow::onTagSearchClicked()
{
    update();
}

void MainWindow::openSourcesDialog()
{
    SourcesDialog* sources = new SourcesDialog(this);
    sources->open();
    sources->run();
    delete sources;
}

void MainWindow::updateTags()
{
    set<string> tags;
    tags = m_index->getAllTags();
    m_allTagsView.update(tags);
}

