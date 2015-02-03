#ifndef __FOTOFING_WINDOW_H_
#define __FOTOFING_WINDOW_H_

#include <map>
#include <string>

#include <gtkmm.h>

#include <fotofing/index.h>

#include "tab.h"

class FotofingWindow : public Gtk::Window, public IndexClient
{
 private:
    Gtk::VBox m_vBox;

    Gtk::MenuBar m_menuBar;
    Gtk::Notebook m_tabs;

    /* *** Status Bar *** */
    Gtk::HBox m_statusBox;
    Gtk::Label m_statusBar;
    Gtk::ProgressBar m_progressBar;
    bool m_progressActive;
    bool progressTimeout();

 public:
    FotofingWindow();
    virtual ~FotofingWindow();

    void openTab(Tab* w);
    void closeTab(Tab* w);
    bool isTabVisible(Gtk::Widget* w);
    void onTabSwitch(Gtk::Widget* w, guint pageNum);
    void updateTabs();

    Tab* getCurrentTab();

    void setStatusMessage(std::string message);
    void startProgress();
    void endProgress();
    void updateProgress(
        int complete,
        int total,
        std::string info);
};

#endif
