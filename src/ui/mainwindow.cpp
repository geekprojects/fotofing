
#include <fotofing/utils.h>

#include "mainwindow.h"
#include "calendarpopup.h"
#include "sourcesdialog.h"
#include "tagdialog.h"

using namespace std;

MainWindow::MainWindow(Index* index) :
    m_dateSeparator("-"),
    m_tagSearchButton("Search"),
    m_photoView(this),
    m_photoDetails(this)
{
    m_index = index;
    m_progressActive = false;
    m_workerThread = NULL;

    set_title("Fotofing");
    set_default_size(600, 400);

    // Toolbar
    m_toolbarTag.set_icon_name("tag-new");
    m_toolbarTag.set_tooltip_text("Add Tag");
    m_toolbarTag.signal_clicked().connect(sigc::mem_fun(
        *this,
        &MainWindow::onTagButton));
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
    m_allTagsView.signal_delete_tags().connect(sigc::mem_fun(
        *this,
        &MainWindow::onDeleteTags));
    m_tagBox.pack_start(m_allTagsView, Gtk::PACK_EXPAND_WIDGET);
    m_tagBox.pack_start(m_tagSearchButton, Gtk::PACK_SHRINK);
    m_tagFrame.add(m_tagBox);

    m_hBox.pack_start(m_tagFrame, Gtk::PACK_SHRINK);
    m_hBox.pack_start(m_photoView, Gtk::PACK_EXPAND_WIDGET);
    m_hBox.pack_start(m_photoDetails, Gtk::PACK_SHRINK);

    m_progressBar.set_show_text(true);
    m_progressBar.set_ellipsize(Pango::ELLIPSIZE_START);
    m_statusBox.pack_start(m_statusBar, Gtk::PACK_EXPAND_WIDGET);
    m_statusBox.pack_start(m_progressBar, Gtk::PACK_SHRINK, 5);

    Gtk::MenuBar* menuBar = createMenu();
    m_vBox.pack_start(*menuBar, Gtk::PACK_SHRINK);
    m_vBox.pack_start(m_toolbarBox, Gtk::PACK_SHRINK);
    m_vBox.pack_start(m_hBox, Gtk::PACK_EXPAND_WIDGET);
    m_vBox.pack_start(m_statusBox, Gtk::PACK_SHRINK);
    add(m_vBox);

    show_all();

    updateSources();
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

void MainWindow::onTagButton()
{
    int res;

    TagDialog* tagDialog = new TagDialog(this);
    res = tagDialog->run();
    if (res == Gtk::RESPONSE_OK)
    {
        string tag = tagDialog->getTag();
        set<string> tags;
        tags.insert(tag);

        vector<Photo*> selected = m_photoView.getSelectedPhotos();
        vector<Photo*>::iterator it;
        for (it = selected.begin(); it != selected.end(); it++)
        {
            m_index->saveTags((*it)->getId(), tags);
        }

        printf("MainWindow::onTagButton: Applying tag %s\n", tag.c_str());
        updateTags();
    }

    delete tagDialog;
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
        m_fromDateButton.set_label(timeToString(m_fromDate, false));
    }
    else
    {
        m_fromDateButton.set_label("-");
    }

    if (m_toDate > 0)
    {
        m_toDateButton.set_label(timeToString(m_toDate, false));
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

void MainWindow::onDeleteTags(vector<Tag*> tags)
{
    int res;

    printf("MainWindow::onDeleteTags: Deleting tags\n");
    res = confirm(
        "Delete selected tags?",
        "Are you sure you wish to delete the selected tags?\n"
        "This cannot be undone.");
    if (res)
    {
        vector<Tag*>::iterator it;
        for (it = tags.begin(); it != tags.end(); it++)
        {
            Tag* tag = *it;
            printf(
                "MainWindow::onDeleteTags: Deleting tag: %s\n",
                tag->getTagName().c_str());
            m_index->removeTag(tag->getTagName());
        }
        updateTags();
    }
}

void MainWindow::openSourcesDialog()
{
    SourcesDialog* sources = new SourcesDialog(this);
    sources->open();
    sources->run();
    delete sources;
    updateSources();
}

void MainWindow::updateTags()
{
    set<string> tags;
    tags = m_index->getAllTags();
    m_allTagsView.update(tags);
    m_photoDetails.updateTags();
}

void MainWindow::updateSources()
{
    if (m_workerThread != NULL)
    {
        // TODO: Queue this!
        printf("MainWindow::updateSources: There's already a worker thread\n");
        return;
    }
    m_workerThread = Glib::Threads::Thread::create(
    sigc::bind(sigc::mem_fun(*this, &MainWindow::updateSourcesThread), this));
}

void MainWindow::updateSourcesThread(MainWindow* arg)
{
    m_index->scanSources(this);
    m_workerThread = NULL;

    updateTags();
    update();
}

bool MainWindow::confirm(string title, string text)
{
    Gtk::MessageDialog dialog(
        *this,
        title,
        false,
        Gtk::MESSAGE_QUESTION,
        Gtk::BUTTONS_OK_CANCEL);
    dialog.set_secondary_text(text);


    int result = dialog.run();

    return (result == Gtk::RESPONSE_OK);
}

void MainWindow::scanProgress(
    Source* source,
    int complete,
    int total,
    std::string info)
{
    m_progressBar.set_fraction((float)complete / (float)total);
    m_progressBar.set_text(info);
}

