

#include "mainwindow.h"
#include "detaildialog.h"

using namespace std;

MainWindow::MainWindow(Index* index) :
    m_tagSearchButton("Search"),
    m_photoDetailPane(Gtk::ORIENTATION_VERTICAL)
{
    m_index = index;

    set_title("Fotofing");
    set_default_size(600, 400);

    // Toolbar
    m_toolbarTag.set_icon_name("tag-new");
    m_toolbar.append(m_toolbarTag);

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

    m_iconView.set_activate_on_single_click(true);
    m_iconView.signal_item_activated().connect(sigc::mem_fun(
        *this,
        &MainWindow::onIconViewItemActivated));

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

    m_vBox.pack_start(m_toolbar, Gtk::PACK_SHRINK);
    m_vBox.pack_start(m_hBox, Gtk::PACK_EXPAND_WIDGET);
    m_vBox.pack_start(m_statusBar, Gtk::PACK_SHRINK);
    add(m_vBox);

    show_all();

    updateTags();
    update();
}

MainWindow::~MainWindow()
{
    freeTags();
}

void MainWindow::update()
{
    m_model->clear();

    vector<Tag*> tags = getSelectedTags();
    vector<Photo*> photos = m_index->getPhotos();
    if (tags.size() > 0)
    {
        vector<string> tagStrings;
        vector<Tag*>::iterator it;
        for (it = tags.begin(); it != tags.end(); it++)
        {
            tagStrings.push_back((*it)->getTagName());
        }
        photos = m_index->getPhotos(tagStrings);
    }
    else
    {
        photos = m_index->getPhotos();
    }

    vector<Photo*>::iterator it;
    for (it = photos.begin(); it != photos.end(); it++)
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
}

void MainWindow::onIconViewItemActivated(const Gtk::TreeModel::Path& path)
{

    Gtk::TreeModel::iterator iter = m_model->get_iter(path);
    if(iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring name = row[m_photoColumns.display_name];
        Photo* photo = row[m_photoColumns.photo];

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
    printf("MainWindow::freeTags: Here!\n");
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

