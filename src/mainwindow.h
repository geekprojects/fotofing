#ifndef __FOTOFING_WINDOW_H_
#define __FOTOFING_WINDOW_H_

#include <map>
#include <string>

#include <gtkmm.h>

#include "index.h"

struct Tag
{
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

class PhotoModelColumns : public Gtk::TreeModelColumnRecord
{
  public:

    Gtk::TreeModelColumn<Glib::ustring> display_name;
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > pixbuf;

    PhotoModelColumns()
    {
        add(display_name);
        add(pixbuf);
    }
};


class MainWindow : public Gtk::Window
{
 private:
    Index* m_index;
    Tag* m_tagRoot;

    Gtk::VBox m_vBox;
    Gtk::HBox m_hBox;

    // Toolbar
    Gtk::Toolbar m_toolbar;
    Gtk::ToolButton m_toolbarTag;

    // Tag VBox
    Gtk::VBox m_tagBox;
    Gtk::Button m_tagSearchButton;

    // Tag View
    const TagModelColumns m_tagColumns;
    Gtk::ScrolledWindow m_scrolledWindowTags;
    Gtk::TreeView m_treeViewTags;
    Glib::RefPtr<Gtk::TreeStore> m_tagTreeStore;
    Glib::RefPtr<Gtk::TreeSelection> m_refTreeSelection;

    // Thumbnail View
    const PhotoModelColumns m_photoColumns;
    Gtk::ScrolledWindow m_scrolledWindowIcons;
    Gtk::IconView m_iconView;
    Glib::RefPtr<Gtk::ListStore> m_model;

    Gtk::Statusbar m_statusBar;

    std::vector<Tag*> getSelectedTags();
    void treeify(Tag* parent, std::string remainder, int level);

    void freeTags();
    void freeThumbnails();

    void onTagRowActivate(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
    void onTagSearchClicked();

 public:
    MainWindow(Index* index);
    virtual ~MainWindow();

    void update();
    void updateTags();
};

#endif
