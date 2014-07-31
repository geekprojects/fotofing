
#include "tagview.h"

using namespace std;

TagView::TagView()
{
    m_tagRoot = NULL;

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

    add(m_treeViewTags);

    // Add signal handler
    m_treeViewTags.signal_row_activated().connect(sigc::mem_fun(
        *this,
        &TagView::onTagRowActivate));
}

TagView::~TagView()
{
    if (m_tagRoot != NULL)
    {
        delete m_tagRoot;
    }
}

void TagView::onTagRowActivate(
    const Gtk::TreeModel::Path& path,
    Gtk::TreeViewColumn* column)
{
    m_rowActivateSignal.emit();
}

void TagView::treeify(Tag* parent, string remainder, int level)
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

vector<Tag*> TagView::getSelectedTags()
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


void TagView::update(std::set<std::string> tags)
{
    if (m_tagRoot != NULL)
    {
        delete m_tagRoot;
    }

    m_tagTreeStore->clear();

    m_tagRoot = new Tag();
    m_tagRoot->name = "";

    set<string>::iterator it1;
    for (it1 = tags.begin(); it1 != tags.end(); it1++)
    {
        string tag = *it1;
        treeify(m_tagRoot, tag, 0);
    }
}

sigc::signal<void>& TagView::signal_row_activate()
{
    return m_rowActivateSignal;
}

