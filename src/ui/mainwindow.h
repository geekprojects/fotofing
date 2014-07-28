#ifndef __FOTOFING_WINDOW_H_
#define __FOTOFING_WINDOW_H_

#include <map>
#include <string>

#include <gtkmm.h>

#include <fotofing/index.h>

#include "photodetails.h"
#include "about.h"

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

class MainWindow : public Gtk::Window
{
 private:
    Index* m_index;
    Tag* m_tagRoot;

    Gtk::VBox m_vBox;
    Gtk::HBox m_hBox;

    Glib::RefPtr<Gtk::Builder> m_refBuilder;

    // Toolbar and Menu
    Gtk::HBox m_toolbarBox;
    Gtk::Toolbar m_toolbar;
    Gtk::ToolButton m_toolbarTag;
    Gtk::ToolButton m_toolbarTagEvent;
    Gtk::ToolButton m_toolbarHide;

    time_t m_fromDate;
    time_t m_toDate;
    Gtk::Button m_fromDateButton;
    Gtk::Button m_toDateButton;
    Gtk::Label m_dateSeparator;

    void onFromDateClicked();
    void onToDateClicked();
    void updateDateButtons();

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
    void onIconViewSelectionChanged();

    /* *** Photo detail panel *** */
    PhotoDetails m_photoDetails;

    Photo* getPhotoFromPath(Gtk::TreePath path);

    /* *** Status Bar *** */
    Gtk::HBox m_statusBox;
    Gtk::Statusbar m_statusBar;
    Gtk::ProgressBar m_progressBar;
    bool m_progressActive;
    bool progressTimeout();

    About m_about;

    Gtk::MenuBar* createMenu();

    void openSourcesDialog();

    std::vector<Tag*> getSelectedTags();
    void treeify(Tag* parent, std::string remainder, int level);

    void freeTags();
    void freePhotos();
    void freeThumbnails();

 public:
    MainWindow(Index* index);
    virtual ~MainWindow();

    Index* getIndex() { return m_index; }

    void update();
    void updateTags();
};

#endif
