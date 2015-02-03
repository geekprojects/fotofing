
#include "ui.h"

FotofingUI::FotofingUI(int argc, char** argv) :
    Gtk::Application(argc, argv, "com.geekprojects.fotofing")
{
    init();
}

FotofingUI::~FotofingUI()
{
}

bool FotofingUI::init()
{

    Glib::set_application_name("Fotofing UI");

    add_action(
        "about",
        sigc::mem_fun(m_about, &About::open));

/*
    actionGroup->add_action(
        "sources",
        sigc::mem_fun(m_library, &Library::openSourcesDialog));

    actionGroup->add_action(
        "edit_export",
        sigc::mem_fun(this, &MainWindow::onExport));

    insert_action_group("fotofing", actionGroup);
*/

    return true;
}

