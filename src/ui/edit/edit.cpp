
#include "edit.h"

using namespace std;

Edit::Edit(MainWindow* mainWindow, Photo* photo)
{
    m_mainWindow = mainWindow;

    pack_start(m_preview, Gtk::PACK_EXPAND_WIDGET);

    vector<File*> files = m_mainWindow->getIndex()->getFiles(photo->getId());
    //m_label.set_label(files.at(0)->getPath());

    m_workflow = new Workflow(photo, files.at(0));
    m_preview.setWorkflow(m_workflow);

    // Available Operations Menu
    m_opsMenuViewStore = Gtk::TreeStore::create(m_opsMenuColumns);
    m_opsMenuView.set_model(m_opsMenuViewStore);
    m_opsMenuView.append_column("Available Operations", m_opsMenuColumns.opText);
    m_opsMenuWindow.add(m_opsMenuView);
    m_opsMenuFrame.set_shadow_type(Gtk::SHADOW_IN);
    m_opsMenuFrame.add(m_opsMenuWindow);
    m_opsBox.pack_start(m_opsMenuFrame, Gtk::PACK_EXPAND_WIDGET);

    Gtk::TreeRow menuRow;
    menuRow = *(m_opsMenuViewStore->append());
    menuRow[m_opsMenuColumns.opText] = "Brightness and Contrast";

    menuRow = *(m_opsMenuViewStore->append());
    menuRow[m_opsMenuColumns.opText] = "Levels";

    menuRow = *(m_opsMenuViewStore->append());
    menuRow[m_opsMenuColumns.opText] = "Refocus";

    // Workflow Operations
    m_opsViewStore = Gtk::TreeStore::create(m_opsColumns);
    m_opsView.set_model(m_opsViewStore);
    m_opsView.set_reorderable();
    m_opsView.append_column("Operation", m_opsColumns.opText);
    m_opsWindow.add(m_opsView);
    m_opsFrame.set_shadow_type(Gtk::SHADOW_IN);
    m_opsFrame.add(m_opsWindow);
    m_opsBox.pack_start(m_opsFrame, Gtk::PACK_EXPAND_WIDGET);
    pack_start(m_opsBox, Gtk::PACK_SHRINK);

    Gtk::TreeRow opRow;
    opRow = *(m_opsViewStore->append());
    opRow[m_opsColumns.opText] = "Brightness and Contrast";

    opRow = *(m_opsViewStore->append());
    opRow[m_opsColumns.opText] = "Another Op";

    opRow = *(m_opsViewStore->append());
    opRow[m_opsColumns.opText] = "Yet Another Op";

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

