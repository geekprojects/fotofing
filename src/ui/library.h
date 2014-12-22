#ifndef __FOTOFING_UI_LIBRARY_H_
#define __FOTOFING_UI_LIBRARY_H_

#include "photodetails.h"
#include "photoview.h"
#include "tagview.h"

class MainWindow;

class Library : public Gtk::VBox, public IndexClient
{
 private:
    MainWindow* m_mainWindow;

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
    PhotoView m_photoView;

    /* *** Photo detail panel *** */
    PhotoDetails m_photoDetails;

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
};

#endif
