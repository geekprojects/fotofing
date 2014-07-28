
#include "sourcesdialog.h"
#include "mainwindow.h"

using namespace std;

SourcesDialog::SourcesDialog(MainWindow* mainWindow) :
    Gtk::Dialog("Sources", *mainWindow, true),
    m_addButton("Add"),
    m_editButton("Edit"),
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
    m_treeView.append_column("Type", m_columns.type);
    m_treeView.append_column("Host", m_columns.host);
    m_treeView.append_column("Path", m_columns.path);

    m_vBox.pack_start(m_hBox);
    m_hBox.pack_start(m_addButton, Gtk::PACK_EXPAND_WIDGET);
    m_hBox.pack_start(m_editButton, Gtk::PACK_EXPAND_WIDGET);
    m_hBox.pack_start(m_deleteButton, Gtk::PACK_EXPAND_WIDGET);

    add_button("_Done", Gtk::RESPONSE_OK);
}

SourcesDialog::~SourcesDialog()
{
}

void SourcesDialog::update()
{

    vector<Source> sources = m_mainWindow->getIndex()->getSources();
    vector<Source>::iterator it;

    m_refListStore->clear();
    for (it = sources.begin(); it != sources.end(); it++)
    {
        Gtk::TreeRow row = *(m_refListStore->append());
        row[m_columns.sourceId] = it->getSourceId();
        row[m_columns.type] = it->getType();
        row[m_columns.host] = it->getHost();
        row[m_columns.path] = it->getPath();
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

