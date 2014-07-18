#ifndef __FOTOFING_WINDOW_H_
#define __FOTOFING_WINDOW_H_

#include <map>
#include <string>

#include <gtkmm.h>

#include "index.h"

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

class PhotoModelColumns : public Gtk::TreeModelColumnRecord
{
  public:

    Gtk::TreeModelColumn<Glib::ustring> display_name;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > pixbuf;
    Gtk::TreeModelColumn<Photo*> photo;

    PhotoModelColumns()
    {
        add(display_name);
        add(pixbuf);
        add(photo);
    }
};

class PhotoPropColumns : public Gtk::TreeModelColumnRecord
{
  public:
    Gtk::TreeModelColumn<Glib::ustring> property;
    Gtk::TreeModelColumn<Glib::ustring> value;

    PhotoPropColumns()
    {
        add(property);
        add(value);
    }
};

class PhotoTagColumns : public Gtk::TreeModelColumnRecord
{
  public:
    Gtk::TreeModelColumn<Glib::ustring> tag;

    PhotoTagColumns()
    {
        add(tag);
    }
};

class MainWindow : public Gtk::Window
{
 private:
    Index* m_index;
    Tag* m_tagRoot;

    Gtk::VBox m_vBox;
    Gtk::HBox m_hBox;

    // Toolbar and Menu
    Gtk::MenuBar m_menuBar;
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
    Gtk::Frame m_tagFrame;

    void onTagRowActivate(
        const Gtk::TreeModel::Path& path,
        Gtk::TreeViewColumn* column);
    void onTagSearchClicked();

    // Thumbnail View
    std::vector<Photo*> m_photos;
    const PhotoModelColumns m_photoColumns;
    Gtk::ScrolledWindow m_scrolledWindowIcons;
    Gtk::IconView m_iconView;
    Glib::RefPtr<Gtk::ListStore> m_model;

    void onIconViewItemActivated(const Gtk::TreeModel::Path& path);

    // Photo detail panel
    Gtk::Paned m_photoDetailPane;

    // Photo properties
    Gtk::ScrolledWindow m_photoPropScrollWindow;
    Gtk::TreeView m_photoPropTreeView;
    Glib::RefPtr<Gtk::ListStore> m_photoPropListStore;
    const PhotoPropColumns m_photoPropColumns;
    Gtk::Frame m_photoPropFrame;

    // Photo tags
    Gtk::ScrolledWindow m_photoTagScrollWindow;
    Gtk::TreeView m_photoTagTreeView;
    Glib::RefPtr<Gtk::ListStore> m_photoTagListStore;
    const PhotoTagColumns m_photoTagColumns;
    Gtk::Frame m_photoTagFrame;

    Gtk::Statusbar m_statusBar;

    void createMenu();

    std::vector<Tag*> getSelectedTags();
    void treeify(Tag* parent, std::string remainder, int level);

    void freeTags();
    void freePhotos();
    void freeThumbnails();

 public:
    MainWindow(Index* index);
    virtual ~MainWindow();

    void update();
    void updateTags();
};

#endif
