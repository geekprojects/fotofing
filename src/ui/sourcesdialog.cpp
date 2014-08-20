
#include "sourcesdialog.h"
#include "mainwindow.h"
#include "uiutils.h"

using namespace std;

SourcesDialog::SourcesDialog(MainWindow* mainWindow) :
    Gtk::Dialog("Sources", *mainWindow, true),
    m_addButton("Add"),
    m_deleteButton("Delete")
{
    m_mainWindow = mainWindow;

    get_content_area()->pack_start(m_vBox, Gtk::PACK_SHRINK);

    m_scrolledWindow.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    m_scrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    m_scrolledWindow.add(m_treeView);
    m_vBox.pack_start(m_scrolledWindow);

    m_refListStore = Gtk::ListStore::create(m_columns);
    m_treeView.set_model(m_refListStore);
    m_treeView.set_rules_hint();

    m_refTreeSelection = m_treeView.get_selection();
    m_refTreeSelection->set_mode(Gtk::SELECTION_SINGLE);

    m_treeView.append_column("Type", m_columns.type);
    m_treeView.append_column("Host", m_columns.host);
    m_treeView.append_column("Path", m_columns.path);

    m_vBox.pack_start(m_hBox);
    m_hBox.pack_start(m_addButton, Gtk::PACK_EXPAND_WIDGET);
    m_hBox.pack_start(m_deleteButton, Gtk::PACK_EXPAND_WIDGET);

    m_addButton.signal_clicked().connect(sigc::mem_fun(
        *this,
        &SourcesDialog::onAddClicked));
    m_deleteButton.signal_clicked().connect(sigc::mem_fun(
        *this,
        &SourcesDialog::onDeleteClicked));

    add_button("_Done", Gtk::RESPONSE_OK);
}

SourcesDialog::~SourcesDialog()
{
}

void SourcesDialog::update()
{
    vector<Source*> sources = m_mainWindow->getIndex()->getSources();
    vector<Source*>::iterator it;

    m_refListStore->clear();
    for (it = sources.begin(); it != sources.end(); it++)
    {
        Gtk::TreeRow row = *(m_refListStore->append());
        row[m_columns.sourceId] = (*it)->getSourceId();
        row[m_columns.type] = (*it)->getType();
        row[m_columns.host] = (*it)->getHost();
        row[m_columns.path] = (*it)->getPath();
        row[m_columns.source] = *(*it);
delete (*it);
    }
}

bool SourcesDialog::getSelectedSource(Source& source)
{
    vector<Gtk::TreeModel::Path> selected;
    selected = m_refTreeSelection->get_selected_rows();

    if (selected.size() > 0)
    {
        Gtk::TreeModel::Path path = selected.at(0);
        Gtk::TreeModel::iterator rowit = m_refListStore->get_iter(path);
        Gtk::TreeRow row = *rowit;
        source = row[m_columns.source];
        return true;
    }
    else
    {
        return false;
    }
}

void SourcesDialog::open()
{
    update();
    show_all();
    present();
}

void SourcesDialog::close(int responseId)
{
    if ((responseId == Gtk::RESPONSE_CLOSE) ||
        (responseId == Gtk::RESPONSE_CANCEL) )
    {
        hide();
    }
}

void SourcesDialog::onAddClicked()
{
    SourceDialog* dialog = new SourceDialog(*this, getHostName(), "");
    int res = dialog->run();

    if (res == Gtk::RESPONSE_OK)
    {
        string type = dialog->getType();
        string host = dialog->getHost();
        string path = dialog->getPath();

        if (type == "File")
        {
            m_mainWindow->getIndex()->addFileSource(path);
            update();
        }
    }

    delete dialog;
}

void SourcesDialog::onDeleteClicked()
{
    Source source;
    bool res = getSelectedSource(source);
    printf("SourcesDialog::onEditClicked: res=%d\n", res);
    if (res)
    {
        bool confirm;
        confirm = UIUtils::confirm(
            *this,
            "Delete Source",
            "Are you sure you want to delete this source?");
        if (confirm)
        {
            m_mainWindow->getIndex()->removeSource(&source);
            // TODO: This should remove all photos belonging to this source
            update();
        }
    }
}

SourceDialog::SourceDialog(Gtk::Window& parent, string host, string path) :
    Gtk::Dialog("Source", parent, true),
    m_typeLabel("Type"),
    m_hostLabel("Host"),
    m_pathLabel("Path"),
    m_pathButton("...")
{
    add_button("_OK", Gtk::RESPONSE_OK);
    add_button("_Cancel", Gtk::RESPONSE_CANCEL);

    m_typeComboBox.append("File");
    m_typeComboBox.set_active_text("File");

    m_grid.attach(m_typeLabel,    0, 0, 1, 1);
    m_grid.attach(m_typeComboBox, 1, 0, 1, 1);

    m_hostEdit.set_text(host);
    m_hostEdit.set_editable(false);
    m_grid.attach(m_hostLabel,    0, 1, 1, 1);
    m_grid.attach(m_hostEdit,     1, 1, 1, 1);

    m_pathBox.pack_start(m_pathEdit, Gtk::PACK_EXPAND_WIDGET);
    m_pathBox.pack_start(m_pathButton, Gtk::PACK_SHRINK);
    m_pathEdit.set_text(path);
    m_grid.attach(m_pathLabel,    0, 2, 1, 1);
    m_grid.attach(m_pathBox,      1, 2, 1, 1);

    m_pathButton.signal_clicked().connect(sigc::mem_fun(
        *this,
        &SourceDialog::onPathButton));

    get_content_area()->pack_start(m_grid, Gtk::PACK_SHRINK);

    show_all();
}

SourceDialog::~SourceDialog()
{
}

string SourceDialog::getType()
{
    return m_typeComboBox.get_active_text();
}

string SourceDialog::getHost()
{
    return m_hostEdit.get_text();
}

string SourceDialog::getPath()
{
    return m_pathEdit.get_text();
}

void SourceDialog::onPathButton()
{
    Gtk::FileChooserDialog dialog(
        "Please choose a folder",
        Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    dialog.set_transient_for(*this);

    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Select", Gtk::RESPONSE_OK);

    int res = dialog.run();

    if (res == Gtk::RESPONSE_OK)
    {
        m_pathEdit.set_text(dialog.get_filename());
    }
}

