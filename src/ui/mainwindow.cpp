
#include <fotofing/utils.h>

#include "dialogs/calendarpopup.h"
#include "dialogs/sourcesdialog.h"
#include "dialogs/tagdialog.h"
#include "edit/edit.h"
#include "mainwindow.h"
#include "uiutils.h"

using namespace std;

MainWindow::MainWindow(Index* index) :
    m_library(this)
{
    m_index = index;
    m_workflowIndex = new WorkflowIndex(index);

    set_title("Fotofing");
    set_default_size(600, 400);

    m_tabs.set_name("ff-content");
    m_tabs.signal_switch_page().connect(sigc::mem_fun(
        *this,
        &MainWindow::onTabSwitch));

    openTab(&m_library);

    m_statusBar.set_justify(Gtk::JUSTIFY_LEFT);
    m_statusBar.set_alignment(Gtk::ALIGN_START);
    m_progressBar.set_show_text(true);
    m_progressBar.set_ellipsize(Pango::ELLIPSIZE_START);
    m_statusBox.pack_start(m_statusBar, Gtk::PACK_EXPAND_WIDGET);
    m_statusBox.pack_start(m_progressBar, Gtk::PACK_SHRINK, 5);

    m_vBox.pack_start(m_menuBar, Gtk::PACK_SHRINK);
    m_vBox.pack_start(m_tabs, Gtk::PACK_EXPAND_WIDGET);
    m_vBox.pack_start(m_statusBox, Gtk::PACK_EXPAND_WIDGET);
    add(m_vBox);

    show_all();

    Glib::RefPtr<Gtk::StyleContext> styleContext = get_style_context();

    Glib::RefPtr<Gtk::CssProvider> refStyleProvider = Gtk::CssProvider::create();

    refStyleProvider->load_from_data(
        "#ff-content GtkIconView { background-color: #080808; }"
        "#ff-content GtkIconView { border: 1px solid #ffffff }"
        "#ff-content GtkIconView.view.cell { color: #ffffff; text-shadow: 1 1 black; }"
        "#ff-content GtkIconView.view.cell:selected { background-color: #101010; background-image: none; text-shadow: 1 1 black; }"
    );

    Gtk::StyleContext::add_provider_for_screen(get_screen(), refStyleProvider, 800);

    initActions();

    m_library.updateSources();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initActions()
{
    Glib::RefPtr<Gio::SimpleActionGroup> actionGroup;
    actionGroup = Gio::SimpleActionGroup::create();
    actionGroup->add_action(
        "about",
        sigc::mem_fun(m_about, &About::open));

    actionGroup->add_action(
        "sources",
        sigc::mem_fun(m_library, &Library::openSourcesDialog));

    actionGroup->add_action(
        "edit_export",
        sigc::mem_fun(this, &MainWindow::onExport));

/*
    actionGroup->add_action(
        "selectAll",
        sigc::mem_fun(m_photoView, &PhotoView::selectAll));
*/

    insert_action_group("fotofing", actionGroup);
}

void MainWindow::editPhoto(Photo* photo)
{
    Workflow* workflow = m_workflowIndex->getWorkflow(photo);
    Edit* editTab = (Edit*)Gtk::manage(new Edit(this, workflow));

    openTab(editTab);
}

void MainWindow::openTab(Tab* w)
{
    int page = m_tabs.append_page(*w, *(w->getTabLabel()));
    m_tabs.set_current_page(page);
}

void MainWindow::closeTab(Tab* w)
{
    m_tabs.remove_page(*w);
}

bool MainWindow::isTabVisible(Gtk::Widget* w)
{
    return (m_tabs.page_num(*w) == m_tabs.get_current_page());
}

void MainWindow::onTabSwitch(Gtk::Widget* w, guint pageNum)
{
    Tab* tab = (Tab*)w;
    printf("MainWindow::onTabSwitch: pageNum=%d\n", pageNum);
    m_menuBar.bind_model(tab->getMenu(), true);
}

void MainWindow::onExport()
{
    Edit* editTab = (Edit*)m_tabs.get_nth_page(m_tabs.get_current_page());
    editTab->onExport();
}

void MainWindow::setStatusMessage(string message)
{
    m_statusBar.set_label(message);
}

void MainWindow::startProgress()
{
    m_progressActive = true;
    m_progressBar.pulse();
    Glib::signal_timeout().connect(sigc::mem_fun(
        *this,
        &MainWindow::progressTimeout), 50 );
}

void MainWindow::endProgress()
{
    m_progressActive = false;
}

void MainWindow::updateProgress(
    int complete,
    int total,
    std::string info)
{
    m_progressBar.set_fraction((float)complete / (float)total);
    m_progressBar.set_text(info);
}

bool MainWindow::progressTimeout()
{
    bool active = m_progressActive;
    if (active)
    {
        m_progressBar.pulse();
    }
    else
    {
        m_progressBar.set_fraction(0.0f);
    }
    return active;
}

