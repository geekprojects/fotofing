
#include <fotofing/utils.h>

#include "dialogs/calendarpopup.h"
#include "dialogs/sourcesdialog.h"
#include "dialogs/tagdialog.h"
#include "mainwindow.h"
#include "uiutils.h"

using namespace std;

Library::Library(MainWindow* mainWindow) :
    Tab(Gtk::ORIENTATION_VERTICAL),
    m_tabLabel("Library"),
    m_dateSeparator("-"),
    m_tagSearchButton("Search"),
    m_photoView(mainWindow->getIndex()),
    m_photoDetails(this)
{
    m_mainWindow = mainWindow;

    //m_progressActive = false;
    m_workerThread = NULL;

    // Toolbar
    m_toolbarTag.set_icon_name("tag-new");
    m_toolbarTag.set_tooltip_text("Add Tag");
    m_toolbarTag.signal_clicked().connect(sigc::mem_fun(
        *this,
        &Library::onTagButton));
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
        &Library::onFromDateClicked));
    m_toDateButton.signal_clicked().connect(sigc::mem_fun(
        *this,
        &Library::onToDateClicked));
    updateDateButtons();

    m_toolbarBox.pack_start(m_toolbar, Gtk::PACK_EXPAND_WIDGET);
    m_toolbarBox.pack_start(m_fromDateButton, Gtk::PACK_SHRINK);
    m_toolbarBox.pack_start(m_dateSeparator, Gtk::PACK_SHRINK);
    m_toolbarBox.pack_start(m_toDateButton, Gtk::PACK_SHRINK);


    m_tagSearchButton.set_tooltip_text("Search for selected tags");
    m_tagSearchButton.signal_clicked().connect(sigc::mem_fun(
        *this,
        &Library::onTagSearchClicked));

    // Tag Tree: Add it
    m_allTagsView.signal_row_activate().connect(sigc::mem_fun(
        *this,
        &Library::update));
    m_allTagsView.signal_delete_tags().connect(sigc::mem_fun(
        *this,
        &Library::onDeleteTags));
    m_tagBox.pack_start(m_allTagsView, Gtk::PACK_EXPAND_WIDGET);
    m_tagBox.pack_start(m_tagSearchButton, Gtk::PACK_SHRINK);
    m_tagFrame.add(m_tagBox);

    m_photoView.signal_cursor_changed().connect(sigc::mem_fun(
        *this,
        &Library::displayDetails));
    m_photoView.signal_activate_photo().connect(sigc::mem_fun(
        m_mainWindow,
        &MainWindow::editPhoto));
    m_photoViewScroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    m_photoViewScroll.add(m_photoView);

    m_hBox.pack_start(m_tagFrame, Gtk::PACK_SHRINK);
    m_hBox.pack_start(m_photoViewScroll, Gtk::PACK_EXPAND_WIDGET);
    m_hBox.pack_start(m_photoDetails, Gtk::PACK_SHRINK);

    pack_start(m_toolbarBox, Gtk::PACK_SHRINK);
    pack_start(m_hBox, Gtk::PACK_EXPAND_WIDGET);

    createMenu();

    show_all();
}

Library::~Library()
{
}

void Library::update()
{
    m_mainWindow->startProgress();

    vector<Tag*> tags = m_allTagsView.getSelectedTags();

    time_t from = m_fromDate;
    time_t to = m_toDate;

    vector<Photo*> photos;
    if (tags.size() > 0)
    {
        vector<string> tagStrings;
        vector<Tag*>::iterator it;
        for (it = tags.begin(); it != tags.end(); it++)
        {
            tagStrings.push_back((*it)->getTagName());
        }
        photos = getIndex()->getPhotos(tagStrings, &from, &to);
    }
    else
    {
        photos = getIndex()->getPhotos(&from, &to);
    }

    m_photoView.update(photos);

    char message[1024];
    if (photos.size() == 1)
    {
        sprintf(message, "Displaying %lu photo", photos.size());
    }
    else
    {
        sprintf(message, "Displaying %lu photos", photos.size());
    }
    getMainWindow()->setStatusMessage(message);

    m_mainWindow->updateProgress(75, 100, "");
    updateDateButtons();

    m_mainWindow->endProgress();
}

void Library::createMenu()
{
/*
    Glib::RefPtr<Gio::SimpleActionGroup> actionGroup;
    actionGroup = Gio::SimpleActionGroup::create();
    actionGroup->add_action(
        "about",
        sigc::mem_fun(m_about, &About::open));

    actionGroup->add_action(
        "sources",
        sigc::mem_fun(m_library, &Library::openSourcesDialog));

    actionGroup->add_action(
        "selectAll",
        sigc::mem_fun(m_photoView, &PhotoView::selectAll));

    insert_action_group("fotofing", actionGroup);
*/

    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create();

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
        "            <attribute name='action' translatable='yes'>app.about</attribute>"
        "          </item>"
        "        </section>"
        "    </submenu>"
        "  </menu>"
        "</interface>";

    builder->add_from_string(ui_info);

    Glib::RefPtr<Glib::Object> object = builder->get_object("main-menu");
    m_menu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
}

void Library::displayDetails(Photo* photo)
{
    m_photoDetails.displayDetails(photo);
}

void Library::onTagButton()
{
    int res;

    TagDialog* tagDialog = new TagDialog(m_mainWindow);
    res = tagDialog->run();
    if (res == Gtk::RESPONSE_OK)
    {
        string tag = tagDialog->getTag();
        std::map<std::string, TagData*> tags;
        tags.insert(make_pair(tag, (TagData*)NULL));

        vector<Photo*> selected = m_photoView.getSelectedPhotos();
        vector<Photo*>::iterator it;

        m_mainWindow->getIndex()->getDatabase()->startTransaction();
        for (it = selected.begin(); it != selected.end(); it++)
        {
            m_mainWindow->getIndex()->saveTags((*it)->getId(), tags);
        }
        m_mainWindow->getIndex()->getDatabase()->endTransaction();

        printf("MainWindow::onTagButton: Applying tag %s\n", tag.c_str());
        updateTags();
    }

    delete tagDialog;
}

void Library::onFromDateClicked()
{
    CalendarPopup* cp = new CalendarPopup(*m_mainWindow, m_fromDate);
    cp->run();

    m_fromDate = cp->getTime();
    delete cp;

    updateDateButtons();
    update();
}

void Library::onToDateClicked()
{
    CalendarPopup* cp = new CalendarPopup(*m_mainWindow, m_toDate);
    cp->run();

    m_toDate = cp->getTime();
    delete cp;

    updateDateButtons();
    update();
}

void Library::updateDateButtons()
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

void Library::onTagSearchClicked()
{
    update();
}

void Library::onDeleteTags(vector<Tag*> tags)
{
    int res;

    printf("Library::onDeleteTags: Deleting tags\n");
    res = UIUtils::confirm(
        *m_mainWindow,
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
                "Library::onDeleteTags: Deleting tag: %s\n",
                tag->getTagName().c_str());
            m_mainWindow->getIndex()->removeTag(tag->getTagName());
        }
        updateTags();
    }
}

void Library::updateTags()
{
    set<string> tags;
    tags = m_mainWindow->getIndex()->getAllTags();
    m_allTagsView.update(tags);
    m_photoDetails.updateTags();
}

void Library::openSourcesDialog()
{
    SourcesDialog* sources = new SourcesDialog(m_mainWindow);
    sources->open();
    sources->run();
    delete sources;
    updateSources();
}

void Library::updateSources()
{
    if (m_workerThread != NULL)
    {
        // TODO: Queue this!
        printf("Library::updateSources: There's already a worker thread\n");
        return;
    }

    m_mainWindow->startProgress();

#if 0
    m_workerThread = Glib::Threads::Thread::create(
    sigc::bind(sigc::mem_fun(*this, &Library::updateSourcesThread), this));
#else
    updateSourcesThread(this);
#endif

    m_mainWindow->endProgress();
}

void Library::updateSourcesThread(Library* arg)
{
    m_mainWindow->getIndex()->scanSources(this);
    m_workerThread = NULL;

    updateTags();
    update();
}

void Library::scanProgress(
    Source* source,
    int complete,
    int total,
    std::string info)
{
    m_mainWindow->updateProgress(complete, total, info);
}

Index* Library::getIndex()
{
    return m_mainWindow->getIndex();
}

