
#include "edit.h"

using namespace std;

Edit::Edit(MainWindow* mainWindow, Workflow* workflow) :
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
            Gtk::Expander* expander = Gtk::manage(new Gtk::Expander());
            Gtk::VBox* attrsBox = Gtk::manage(new Gtk::VBox());

            vector<OperationAttribute> attrs;
            attrs = op->getOperation()->getAttributes();
            vector<OperationAttribute>::iterator attrIt;
            for (attrIt = attrs.begin(); attrIt != attrs.end(); attrIt++)
            {
                OperationAttribute attr = *attrIt;
                Gtk::Label* attrLabel = Gtk::manage(new Gtk::Label(attr.label));
                attrsBox->pack_start(*attrLabel, Gtk::PACK_SHRINK);

                if (attr.type == OPERATION_ATTR_TYPE_INT)
                {
                    Glib::RefPtr<Gtk::Adjustment> adjustment(
                        Gtk::Adjustment::create(
                            attr.def.i,
                            attr.min.i,
                            attr.max.i,
                            1,
                            10.0,
                            0.0) );
                    Gtk::Scale* attrScale = Gtk::manage(new Gtk::Scale(adjustment, Gtk::ORIENTATION_HORIZONTAL));
                    attrScale->set_digits(0);

                    attrScale->set_value_pos(Gtk::POS_TOP);
                    attrScale->set_draw_value();
                    adjustment->signal_value_changed().connect(sigc::mem_fun(*this, &Edit::onAttrAdjustmentChanged));
                    attrsBox->pack_start(*attrScale, Gtk::PACK_SHRINK);
                }
                else if (attr.type == OPERATION_ATTR_TYPE_DOUBLE)
                {
                    Glib::RefPtr<Gtk::Adjustment> adjustment(
                        Gtk::Adjustment::create(
                            attr.def.d,
                            attr.min.d,
                            attr.max.d,
                            0.01,
                            10.0,
                            0.0) );
                    Gtk::Scale* attrScale = Gtk::manage(new Gtk::Scale(adjustment, Gtk::ORIENTATION_HORIZONTAL));
                    attrScale->set_digits(2);

                    attrScale->set_value_pos(Gtk::POS_TOP);
                    attrScale->set_draw_value();
                    attrsBox->pack_start(*attrScale, Gtk::PACK_SHRINK);
                }
            }

            expander->set_label(op->getOperation()->getName());
            expander->add(*attrsBox);
            expander->show_all();
            m_opsBox.pack_start(*expander, Gtk::PACK_SHRINK);
        }
    }

    m_preview.render(true);
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

void Edit::onAttrAdjustmentChanged()
{
    printf("Edit::onAttrAdjustmentChanged: Here!\n");
}

