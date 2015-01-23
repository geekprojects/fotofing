
#include "edit.h"

using namespace std;

Edit::Edit(MainWindow* mainWindow, Workflow* workflow)
{
    m_mainWindow = mainWindow;

    pack_start(m_preview, Gtk::PACK_EXPAND_WIDGET);

    m_workflow = workflow;
    m_preview.setWorkflow(m_workflow);

    // Available Operations Menu
    m_opsMenuViewStore = Gtk::TreeStore::create(m_opsMenuColumns);
    m_opsMenuView.set_model(m_opsMenuViewStore);
    m_opsMenuView.append_column("Available Operations", m_opsMenuColumns.opText);
    m_opsMenuSelection = m_opsMenuView.get_selection();
    m_opsMenuWindow.add(m_opsMenuView);
    m_opsMenuFrame.set_shadow_type(Gtk::SHADOW_IN);
    m_opsMenuFrame.add(m_opsMenuWindow);
    m_opsBox.pack_start(m_opsMenuFrame, Gtk::PACK_EXPAND_WIDGET);

    m_opsMenuView.signal_row_activated().connect(sigc::mem_fun(
        *this,
        &Edit::onOpsMenuRowActivate));

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

    m_tabLabelText.set_label("Edit " + workflow->getPhoto()->getId().substr(0, 6));
    m_tabLabelClose.set_relief(Gtk::RELIEF_NONE);
    m_tabLabelClose.set_border_width(0);
    m_tabLabelClose.set_image_from_icon_name(
        "window-close",
        Gtk::ICON_SIZE_MENU);

    m_tabLabel.pack_start(m_tabLabelText, Gtk::PACK_EXPAND_WIDGET);
    m_tabLabel.pack_start(m_tabLabelClose, Gtk::PACK_SHRINK);
    m_tabLabel.show_all();

    updateOperations();
    updateWorkflow();

    show_all();
}

Edit::~Edit()
{
}

void Edit::updateOperations()
{
    m_opsMenuViewStore->clear();

    vector<FotofingPlugin*> operations = FotofingPlugin::getPlugins("operation");
    vector<FotofingPlugin*>::iterator it;
    for (it = operations.begin(); it != operations.end(); it++)
    {
        Operation* op = (Operation*)*it;

        Gtk::TreeRow menuRow;
        menuRow = *(m_opsMenuViewStore->append());
        menuRow[m_opsMenuColumns.opText] = op->getName();
        menuRow[m_opsMenuColumns.operation] = op;
    }
}

void Edit::updateWorkflow()
{
    m_opsViewStore->clear();
)
    vectoOperationInstance*>::iteratornt v4si __attribute__ ((vector_size (16)));
<<8) it;
    for (
        it = m_workflow->getOperations().begin();
        it != m_workflow->getOperations().end();
        it++)
    {
        OperationInstance* op = *it;
        Gtk::TreeRow opRow;
        opRow = *(m_opsViewStore->append());
        opRow[m_opsColumns.opText] = op->getOperation()->getName().c_str();
    }

    m_preview.render();
}

void Edit::onOpsMenuRowActivate(
    const Gtk::TreeModel::Path& path,
    Gtk::TreeViewColumn* column)
{
    vector<Gtk::TreeModel::Path> selected;
    selected = m_opsMenuSelection->get_selected_rows();

    if (selected.size() == 1)
    {
        vector<Gtk::TreeModel::Path>::iterator it = selected.begin();
        Gtk::TreeModel::Path path = *it;
        Gtk::TreeModel::iterator rowit = m_opsMenuViewStore->get_iter(path);
        Gtk::TreeRow row = *rowit;
        Operation* op = row[m_opsMenuColumns.operation];
        Glib::ustring name = row[m_opsMenuColumns.opText];
        printf("Edit::onOpsMenuRowActivate: selected: %s\n", op->getName().c_str());
        m_workflow->addOperation(op->createInstance());

       updateWorkflow();
    }
}

