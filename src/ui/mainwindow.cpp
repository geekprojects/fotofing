
#include <fotofing/utils.h>

#include "mainwindow.h"
#include "calendarpopup.h"

using namespace std;

MainWindow::MainWindow(Index* index) :
    m_dateSeparator("-"),
    m_tagSearchButton("Search"),
    m_photoDetailPane(Gtk::ORIENTATION_VERTICAL)
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


    // Tag tree
    m_tagTreeStore = Gtk::TreeStore::create(m_tagColumns);
    m_treeViewTags.set_model(m_tagTreeStore);
    m_treeViewTags.set_rules_hint();

    m_refTreeSelection = m_treeViewTags.get_selection();
    m_refTreeSelection->set_mode(Gtk::SELECTION_MULTIPLE);

    // Tag tree: Column
    int cols_count = m_treeViewTags.append_column("Tag", m_tagColumns.tagText);
    Gtk::TreeViewColumn* pColumn = m_treeViewTags.get_column(cols_count-1);
    Gtk::CellRenderer* pRenderer = pColumn->get_first_cell();
    pRenderer->property_xalign().set_value(0.0);
    pColumn->set_clickable();

    // Add signal handler
    m_treeViewTags.signal_row_activated().connect(sigc::mem_fun(
        *this,
        &MainWindow::onTagRowActivate));
    m_tagSearchButton.set_tooltip_text("Search for selected tags");
    m_tagSearchButton.signal_clicked().connect(sigc::mem_fun(
        *this,
        &MainWindow::onTagSearchClicked));

    // Tag Tree: Add it
    m_scrolledWindowTags.add(m_treeViewTags);
    m_tagBox.pack_start(m_scrolledWindowTags, Gtk::PACK_EXPAND_WIDGET);
    m_tagBox.pack_start(m_tagSearchButton, Gtk::PACK_SHRINK);
    m_tagFrame.add(m_tagBox);
    m_hBox.pack_start(m_tagFrame, Gtk::PACK_SHRINK);

    // Thumbnail View
    m_model = Gtk::ListStore::create(m_photoColumns);
    //m_model->set_sort_column(Gtk::TreeSortable::DEFAULT_SORT_COLUMN_ID, Gtk::SORT_ASCENDING);

    m_iconView.set_model(m_model);
    m_iconView.set_selection_mode(Gtk::SELECTION_MULTIPLE);

    m_iconView.set_text_column(m_photoColumns.display_name);
    m_iconView.set_pixbuf_column(m_photoColumns.pixbuf);

    m_iconView.set_item_width(10);
    m_iconView.grab_focus();

    m_iconView.set_activate_on_single_click(false);
    m_iconView.signal_item_activated().connect(sigc::mem_fun(
        *this,
        &MainWindow::onIconViewItemActivated));
    m_iconView.signal_selection_changed().connect(sigc::mem_fun(
        *this,
        &MainWindow::onIconViewSelectionChanged));


    m_scrolledWindowIcons.add(m_iconView);
    m_hBox.pack_start(m_scrolledWindowIcons, Gtk::PACK_EXPAND_WIDGET);

    /* *** Photo Detail Box *** */

    // Photo properties
    m_photoPropListStore = Gtk::ListStore::create(m_photoPropColumns);
    m_photoPropTreeView.set_model(m_photoPropListStore);
    m_photoPropTreeView.append_column_editable("Property", m_photoPropColumns.property);
    m_photoPropTreeView.append_column_editable("Value", m_photoPropColumns.value);

    m_photoPropScrollWindow.add(m_photoPropTreeView);
    m_photoPropFrame.set_shadow_type(Gtk::SHADOW_IN);
    m_photoPropFrame.add(m_photoPropScrollWindow);
    m_photoDetailPane.pack1(m_photoPropFrame, true, false);

    // Photo tags
    m_photoTagListStore = Gtk::ListStore::create(m_photoTagColumns);
    m_photoTagTreeView.set_model(m_photoTagListStore);
    m_photoTagScrollWindow.add(m_photoTagTreeView);
    m_photoTagTreeView.append_column_editable("Tag", m_photoTagColumns.tag);

    m_photoTagFrame.set_shadow_type(Gtk::SHADOW_IN);
    m_photoTagFrame.add(m_photoTagScrollWindow);
    m_photoDetailPane.pack2(m_photoTagFrame, true, false);

    m_hBox.pack_start(m_photoDetailPane, Gtk::PACK_SHRINK);

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
    delete m_tagRoot;
    freePhotos();
}

Gtk::MenuBar* MainWindow::createMenu()
{

    Glib::RefPtr<Gio::SimpleActionGroup> actionGroup;
    actionGroup = Gio::SimpleActionGroup::create();

    actionGroup->add_action(
        "about",
        sigc::mem_fun(m_about, &About::open));

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

    m_model->clear();

    freePhotos();

    vector<Tag*> tags = getSelectedTags();
    if (tags.size() > 0)
    {
        vector<string> tagStrings;
        vector<Tag*>::iterator it;
        for (it = tags.begin(); it != tags.end(); it++)
        {
            tagStrings.push_back((*it)->getTagName());
        }
        m_photos = m_index->getPhotos(tagStrings, &m_fromDate, &m_toDate);
    }
    else
    {
        m_photos = m_index->getPhotos(&m_fromDate, &m_toDate);
    }

    //m_progressBar.set_fraction(0.75f);
    updateDateButtons();

    vector<Photo*>::iterator it;
    for (it = m_photos.begin(); it != m_photos.end(); it++)
    {
        Gtk::TreeModel::iterator iter = m_model->append();
        Gtk::TreeModel::Row row = *iter;

        Photo* photo = *it;
        Surface* thumbnail = photo->getThumbnail();

        Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data(
            thumbnail->getData(),
            Gdk::COLORSPACE_RGB,
            true,
            8,
            thumbnail->getWidth(),
            thumbnail->getHeight(),
            thumbnail->getWidth() * 4
        );

        string displayName = photo->getId().substr(0, 6) + "...";
        row[m_photoColumns.display_name] = displayName;
        row[m_photoColumns.pixbuf] = pixbuf;
        //row[m_photoColumns.photo] = Glib::RefPtr<Photo>(photo);
        row[m_photoColumns.photo] = photo;

        //delete photo;
    }
    //m_progressBar.set_fraction(1.0f);
    m_progressActive = false;
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
    m_fromDateButton.set_label(timeToString(m_fromDate));
    m_toDateButton.set_label(timeToString(m_toDate));
}


void MainWindow::onIconViewItemActivated(const Gtk::TreeModel::Path& path)
{
    Photo* photo = getPhotoFromPath(path);
    displayDetails(photo);

    vector<File*> files = m_index->getFiles(photo->getId());
    if (files.size() > 0)
    {
        // TODO: Make this configurable!
        string cmd = string("/usr/bin/qiv -fm ") + files.at(0)->getPath();
        printf("MainWindow::onIconViewItemActivated: cmd=%s\n", cmd.c_str());
        int res;
        res = system(cmd.c_str());
        printf("MainWindow::onIconViewItemActivated: res=%d\n", res);
    }
}

void MainWindow::onIconViewSelectionChanged()
{
    vector<Gtk::TreePath> selected = m_iconView.get_selected_items();
    if (selected.size() > 0)
    {
        displayDetails(getPhotoFromPath(selected.at(0)));
    }
}

Photo* MainWindow::getPhotoFromPath(Gtk::TreePath path)
{
    Gtk::TreeModel::iterator iter = m_model->get_iter(path);
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        return row[m_photoColumns.photo];
    }
    return NULL;
}

void MainWindow::displayDetails(Photo* photo)
{
    m_photoPropListStore->clear();
    Gtk::TreeRow propRow;
    propRow = *(m_photoPropListStore->append());
    propRow[m_photoPropColumns.property] = "Id";
    propRow[m_photoPropColumns.value] = photo->getId();

    time_t ts = photo->getTimestamp();
    struct tm tm;
    localtime_r(&ts, &tm);

    char tsbuf[64];
    strftime(tsbuf, 64, "%x", &tm);
    propRow = *(m_photoPropListStore->append());
    propRow[m_photoPropColumns.property] = "Date";
    propRow[m_photoPropColumns.value] = string(tsbuf);

    strftime(tsbuf, 64, "%X", &tm);
    propRow = *(m_photoPropListStore->append());
    propRow[m_photoPropColumns.property] = "Time";
    propRow[m_photoPropColumns.value] = string(tsbuf);

    // Tags
    m_photoTagListStore->clear();
    set<string> tags = photo->getTags();
    if (tags.size() == 0)
    {
        // Only do this if we haven't already retrieved the tags
        tags = m_index->getTags(photo->getId());
        photo->setTags(tags);
    }

    set<string>::iterator tagIt;
    for (tagIt = tags.begin(); tagIt != tags.end(); tagIt++)
    {
        Gtk::TreeRow tagRow;
        tagRow = *(m_photoTagListStore->append());
        tagRow[m_photoTagColumns.tag] = *tagIt;
    }
}

void MainWindow::onTagRowActivate(
    const Gtk::TreeModel::Path& path,
    Gtk::TreeViewColumn* column)
{
    update();
}

void MainWindow::onTagSearchClicked()
{
    update();
}

vector<Tag*> MainWindow::getSelectedTags()
{
    vector<Tag*> tags;

    vector<Gtk::TreeModel::Path> selected;
    selected = m_refTreeSelection->get_selected_rows();

    vector<Gtk::TreeModel::Path>::iterator it;
    for (it = selected.begin(); it != selected.end(); it++)
    {
        Gtk::TreeModel::Path path = *it;
        Gtk::TreeModel::iterator rowit = m_tagTreeStore->get_iter(path);
        Gtk::TreeRow row = *rowit;
        Tag* tag = row[m_tagColumns.tag];
        tags.push_back(tag);
    }
    return tags;
}

void MainWindow::treeify(Tag* parent, string remainder, int level)
{
    int i;
    string spaces = "";
    for (i = 0; i < level; i++)
    {
        spaces += "    ";
    }

    size_t pos = remainder.find('/');
    string part = remainder;
    if (pos != remainder.npos)
    {
        part = remainder.substr(0, pos);
    }

    Tag* tag;

    std::map<std::string,Tag*>::iterator it;

    it = parent->children.find(part);
    if (it != parent->children.end())
    {
        tag = it->second;
    }
    else
    {
        tag = new Tag();
        tag->parent = parent;
        tag->name = part;
        parent->children.insert(make_pair(part, tag));
        if (parent->treeRow == NULL)
        {
            tag->treeRow = *(m_tagTreeStore->append());
        }
        else
        {
            tag->treeRow = *(m_tagTreeStore->append(parent->treeRow.children()));
        }
        tag->treeRow[m_tagColumns.tagText] = part;
        tag->treeRow[m_tagColumns.tag] = tag;
    }

    if (pos != remainder.npos)
    {
        treeify(tag, remainder.substr(pos + 1), level + 1);
    }
}

void MainWindow::freeTags()
{
}

void MainWindow::freePhotos()
{
    vector<Photo*>::iterator it;
    for (it = m_photos.begin(); it != m_photos.end(); it++)
    {
        delete *it;
    }
    m_photos.clear();
}

void MainWindow::updateTags()
{
    set<string> tags;
    tags = m_index->getAllTags();

    m_tagRoot = new Tag();
    m_tagRoot->name = "";

    set<string>::iterator it1;
    for (it1 = tags.begin(); it1 != tags.end(); it1++)
    {
        string tag = *it1;
        treeify(m_tagRoot, tag, 0);
    }
}

