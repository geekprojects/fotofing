
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
    delete m_tagRoot;
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

    vector<Tag*> tags = getSelectedTags();
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

void MainWindow::openSourcesDialog()
{
    SourcesDialog* sources = new SourcesDialog(this);
    sources->open();
    sources->run();
    delete sources;
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

