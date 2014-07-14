

#include "mainwindow.h"

using namespace std;

MainWindow::MainWindow(Index* index)
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

    Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeViewTags.get_selection();
    refTreeSelection->set_mode(Gtk::SELECTION_MULTIPLE);

    // Tag tree: Column
    int cols_count = m_treeViewTags.append_column("Tag", m_tagColumns.tag);
    Gtk::TreeViewColumn* pColumn = m_treeViewTags.get_column(cols_count-1);
    Gtk::CellRenderer* pRenderer = pColumn->get_first_cell();
    pRenderer->property_xalign().set_value(0.0);
    pColumn->set_clickable();

    // Tag Tree: Add it
    m_scrolledWindowTags.add(m_treeViewTags);
    m_hBox.pack_start(m_scrolledWindowTags, Gtk::PACK_SHRINK);

    m_model = Gtk::ListStore::create(m_photoColumns);
    //m_model->set_sort_column(Gtk::TreeSortable::DEFAULT_SORT_COLUMN_ID, Gtk::SORT_ASCENDING);

    m_iconView.set_model(m_model);
    m_iconView.set_selection_mode(Gtk::SELECTION_MULTIPLE);

    m_iconView.set_text_column(m_photoColumns.display_name);
    m_iconView.set_pixbuf_column(m_photoColumns.pixbuf);

    m_iconView.set_item_width(10);
    m_iconView.grab_focus();

    m_scrolledWindowIcons.add(m_iconView);
    m_hBox.pack_start(m_scrolledWindowIcons, Gtk::PACK_EXPAND_WIDGET);

    m_vBox.pack_start(m_toolbar, Gtk::PACK_SHRINK);
    m_vBox.pack_start(m_hBox, Gtk::PACK_EXPAND_WIDGET);
    m_vBox.pack_start(m_statusBar, Gtk::PACK_SHRINK);
    add(m_vBox);

    //m_iconView.set_columns(-1);

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

    vector<Photo*> photos = m_index->getPhotos();
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

        //delete photo;
    }
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

    printf("treeify: -> %s%s\n", spaces.c_str(), part.c_str());

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
        tag->treeRow[m_tagColumns.tag] = part;
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
    m_tagRoot->name = "ROOT";

    set<string>::iterator it1;
    for (it1 = tags.begin(); it1 != tags.end(); it1++)
    {
        string tag = *it1;
        treeify(m_tagRoot, tag, 0);
    }
}

