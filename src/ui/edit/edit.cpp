
#include "edit.h"
#include "operationwidget.h"

using namespace Geek::Gfx;
using namespace std;

Edit::Edit(MainWindow* mainWindow, Workflow* workflow) :
    Tab(Gtk::ORIENTATION_HORIZONTAL),
    m_preview(this)
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
    m_workflowBox.pack_start(m_opsMenuFrame, Gtk::PACK_EXPAND_WIDGET);

    m_opsMenuView.signal_row_activated().connect(sigc::mem_fun(
        *this,
        &Edit::onOpsMenuRowActivate));

    // Workflow Operations
    m_opsWindow.add(m_opsBox);
    m_opsFrame.set_shadow_type(Gtk::SHADOW_IN);
    m_opsFrame.add(m_opsWindow);
    m_workflowBox.pack_start(m_opsFrame, Gtk::PACK_EXPAND_WIDGET);

    pack_start(m_workflowBox, Gtk::PACK_SHRINK);

    m_tabLabelText.set_label("Edit " + workflow->getPhoto()->getId().substr(0, 6));
    m_tabLabelClose.set_relief(Gtk::RELIEF_NONE);
    m_tabLabelClose.set_border_width(0);
    m_tabLabelClose.set_image_from_icon_name(
        "window-close",
        Gtk::ICON_SIZE_MENU);
    m_tabLabelClose.signal_clicked().connect(
        sigc::mem_fun(*this, &Edit::close));

    m_tabLabel.pack_start(m_tabLabelText, Gtk::PACK_EXPAND_WIDGET);
    m_tabLabel.pack_start(m_tabLabelClose, Gtk::PACK_SHRINK);
    m_tabLabel.show_all();

    updateOperations();
    updateWorkflow();

    createMenu();

    show_all();
}

Edit::~Edit()
{
    printf("Edit::~Edit: Here!\n");
    if (m_workflow != NULL)
    {
        delete m_workflow;
    }
}

void Edit::createMenu()
{
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create();

    Glib::ustring ui_info =
        "<interface>"
        "  <menu id='main-menu'>"
        "    <submenu>"
        "      <attribute name='label' translatable='yes'>_File</attribute>"
        "        <section>"
        "          <item>"
        "            <attribute name='label' translatable='yes'>Export...</attribute>"
        "            <attribute name='action' translatable='yes'>fotofing.edit_export</attribute>"
        "          </item>"
        "        </section>"
        "    </submenu>"
        "    <submenu>"
        "      <attribute name='label' translatable='yes'>_Help</attribute>"
        "        <section>"
        "          <item>"
        "            <attribute name='label' translatable='yes'>About</attribute>"
        "            <attribute name='action' translatable='yes'>fotofing.about</attribute>"
        "          </item>"
        "        </section>"
        "    </submenu>"
        "  </menu>"
        "</interface>";

    builder->add_from_string(ui_info);

    Glib::RefPtr<Glib::Object> object = builder->get_object("main-menu");
    m_menu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
}

void Edit::close()
{
    m_mainWindow->closeTab(this);
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
    m_mainWindow->getWorkflowIndex()->saveWorkflow(m_workflow);

    // Clear the VBox. THere's got to be a better way?
    vector<Gtk::Widget*> children = m_opsBox.get_children();
    vector<Gtk::Widget*>::iterator it;
    for (it = children.begin(); it != children.end(); it++)
    {
        Gtk::Widget* w = *it;
        m_opsBox.remove(*w);
    }

    if (m_workflow->getOperations().size() == 0)
    {
        Gtk::Label* label = Gtk::manage(new Gtk::Label("No operations"));
        m_opsBox.pack_start(*label, Gtk::PACK_SHRINK);
    }
    else
    {
        vector<OperationInstance*>::iterator it;
        for (
            it = m_workflow->getOperations().begin();
            it != m_workflow->getOperations().end();
            it++)
        {
            OperationInstance* op = *it;

            OperationWidget* opWidget = Gtk::manage(
                new OperationWidget(this, op));
            m_opsBox.pack_start(*opWidget, Gtk::PACK_SHRINK);
        }
    }

    m_preview.render(true);
}

void Edit::deleteOperation(OperationInstance* op)
{
    m_workflow->deleteOperation(op);
    updateWorkflow();
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

void Edit::onAttrChanged()
{
    m_preview.render(true);
    m_mainWindow->getWorkflowIndex()->saveWorkflow(m_workflow);
}

void Edit::onExport()
{
    Gtk::FileChooserDialog dialog("Export...",
        Gtk::FILE_CHOOSER_ACTION_SAVE);
    //dialog.set_transient_for(*this);

  Glib::RefPtr<Gtk::FileFilter> filter_cpp = Gtk::FileFilter::create();
  filter_cpp->set_name("JPEG Image");
  filter_cpp->add_mime_type("image/jpeg");
  dialog.add_filter(filter_cpp);


    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Export", Gtk::RESPONSE_OK);

    int result;
    result = dialog.run();
    if (result == Gtk::RESPONSE_OK)
    {
        string filename = dialog.get_filename();
        if (filename.find('.') == filename.npos)
        {
            filename = filename += ".jpg";
        }
        printf("Edit::onExport: Exporting to %s\n", filename.c_str());

        Surface* image = Surface::loadJPEG(m_workflow->getFile()->getPath());

        vector<OperationInstance*>::iterator it;
        for (
            it = m_workflow->getOperations().begin();
            it != m_workflow->getOperations().end();
            it++)
        {
            OperationInstance* op = *it;
            op->apply(image, NULL);
        }
        image->saveJPEG(filename);
        delete image;
    }
}

