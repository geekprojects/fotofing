
#include "edit.h"

using namespace std;

Edit::Edit(MainWindow* mainWindow, Photo* photo)
{
    m_mainWindow = mainWindow;

    pack_start(m_label, Gtk::PACK_SHRINK);
    pack_start(m_preview, Gtk::PACK_EXPAND_WIDGET);

    vector<File*> files = m_mainWindow->getIndex()->getFiles(photo->getId());
    m_label.set_label(files.at(0)->getPath());

    m_workflow = new Workflow(photo, files.at(0));
    m_preview.setWorkflow(m_workflow);

    m_tabLabelText.set_label("Edit " + photo->getId().substr(0, 6));
    m_tabLabelClose.set_relief(Gtk::RELIEF_NONE);
    m_tabLabelClose.set_border_width(0);
    m_tabLabelClose.set_image_from_icon_name(
        "window-close",
        Gtk::ICON_SIZE_MENU);

    m_tabLabel.pack_start(m_tabLabelText, Gtk::PACK_EXPAND_WIDGET);
    m_tabLabel.pack_start(m_tabLabelClose, Gtk::PACK_SHRINK);
    m_tabLabel.show_all();

    show_all();


}

Edit::~Edit()
{
}

