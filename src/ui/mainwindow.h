#ifndef __FOTOFING_WINDOW_H_
#define __FOTOFING_WINDOW_H_

#include <map>
#include <string>

#include <gtkmm.h>

#include <fotofing/index.h>

#include "photodetails.h"
#include "photoview.h"
#include "tagview.h"
#include "about.h"

class MainWindow : public Gtk::Window
{
 private:
    Index* m_index;

    Gtk::VBox m_vBox;
    Gtk::HBox m_hBox;

    Glib::RefPtr<Gtk::Builder> m_refBuilder;

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

    /* *** Status Bar *** */
    Gtk::HBox m_statusBox;
    Gtk::Statusbar m_statusBar;
    Gtk::ProgressBar m_progressBar;
    bool m_progressActive;
    bool progressTimeout();

    About m_about;

    Gtk::MenuBar* createMenu();

    void openSourcesDialog();

    bool confirm(std::string title, std::string text);

 public:
    MainWindow(Index* index);
    virtual ~MainWindow();

    Index* getIndex() { return m_index; }

    void update();
    void updateTags();

    void displayDetails(Photo* photo);
};

#endif
