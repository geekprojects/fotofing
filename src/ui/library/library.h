#ifndef __FOTOFING_UI_LIBRARY_H_
#define __FOTOFING_UI_LIBRARY_H_

#include <fotofing/index.h>

#include "tab.h"
#include "photodetails.h"
#include "photoview2.h"
#include "widgets/tagview.h"

class MainWindow;
class PhotoView;

class Library : public Tab, public IndexClient
{
 private:
    MainWindow* m_mainWindow;

    Gtk::Label m_tabLabel;
    Glib::RefPtr<Gio::Menu> m_menu;
    Gtk::HBox m_hBox;

    // Toolbar and Menu
    Gtk::HBox m_toolbarBox;
    Gtk::Toolbar m_toolbar;
    Gtk::ToolButton m_toolbarTag;
    Gtk::ToolButton m_toolbarTagEvent;
    Gtk::ToolButton m_toolbarHide;

    void onTagButton();

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
    TagView m_allTagsView;
    Gtk::Button m_tagSearchButton;

    // Tag View
    Gtk::Frame m_tagFrame;
    void onTagSearchClicked();
    void onDeleteTags(std::vector<Tag*> tags);

    // Thumbnail View
Gtk::ScrolledWindow m_photoViewScroll;
PhotoView2 m_photoView;

    /* *** Photo detail panel *** */
    PhotoDetails m_photoDetails;

    void createMenu();

 public:
    Library(MainWindow* mainWindow);
    ~Library();

    void update();
    void updateTags();
    void updateSources();

    void openSourcesDialog();

    Glib::Threads::Thread* m_workerThread;
    void updateSourcesThread(Library* arg);
    void updateThread(MainWindow* arg);

    virtual void scanProgress(
        Source* source,
        int complete,
        int total,
        std::string info);

    void displayDetails(Photo* photo);

    MainWindow* getMainWindow() { return m_mainWindow; }
    Index* getIndex();

    Gtk::Widget* getTabLabel() { return &m_tabLabel; }
    Glib::RefPtr< Gio::Menu >& getMenu() { return m_menu; }
};

#endif
