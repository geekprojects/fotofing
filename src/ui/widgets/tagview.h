#ifndef __FOTOFING_UI_TAGVIEW_H_
#define __FOTOFING_UI_TAGVIEW_H_

#include <string>
#include <vector>
#include <set>
#include <map>

#include <gtkmm.h>

struct Tag
{
    Tag()
    {
        parent = NULL;
    }

    ~Tag()
    {
        std::map<std::string, Tag*>::iterator it;
        for (it = children.begin(); it != children.end(); it++)
        {
            delete it->second;
        }
    }

    Tag* parent;
    std::string name;
    std::map<std::string, Tag*> children;
    Gtk::TreeRow treeRow;

    std::string getTagName()
    {
        std::string fullname = "";
        if (parent != NULL)
        {
            if (parent->parent != NULL)
            {
                fullname = parent->getTagName() + "/";
            }
        }
        fullname += name;
        return fullname;
    }
};

class TagModelColumns : public Gtk::TreeModelColumnRecord
{
 public:
    Gtk::TreeModelColumn<Glib::ustring> tagText;
    Gtk::TreeModelColumn<Tag*> tag;

    TagModelColumns()
    {
        add(tagText);
        add(tag);
    }
};

class TagView : public Gtk::ScrolledWindow
{
 private:
    Tag* m_tagRoot;

    sigc::signal<void> m_rowActivateSignal;
    sigc::signal<void, std::vector<Tag*> > m_deleteTagsSignal;

    const TagModelColumns m_tagColumns;
    Gtk::ScrolledWindow m_scrolledWindowTags;
    Gtk::TreeView m_treeViewTags;
    Glib::RefPtr<Gtk::TreeStore> m_tagTreeStore;
    Glib::RefPtr<Gtk::TreeSelection> m_refTreeSelection;

    Gtk::Menu m_popupMenu;

    void onTagRowActivate(
        const Gtk::TreeModel::Path& path,
        Gtk::TreeViewColumn* column);

    bool onButtonPress(GdkEventButton* event);

    void treeify(Tag* parent, std::string remainder, int level);

    void freeTags();

 public:
    TagView();
    ~TagView();

    std::vector<Tag*> getSelectedTags();

    void update(std::set<std::string> tags);

    void deleteTags();
    void aliasTag();

    sigc::signal<void>& signal_row_activate();
    sigc::signal<void, std::vector<Tag*> >& signal_delete_tags();

};

#endif
