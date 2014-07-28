#ifndef __FOTOFING_UI_SOURCESDIALOG_H_
#define __FOTOFING_UI_SOURCESDIALOG_H_

#include <gtkmm.h>

class MainWindow;

struct SourcesColumns : public Gtk::TreeModelColumnRecord
{
    Gtk::TreeModelColumn<int64_t> sourceId;
    Gtk::TreeModelColumn<Glib::ustring> type;
    Gtk::TreeModelColumn<Glib::ustring> host;
    Gtk::TreeModelColumn<Glib::ustring> path;

    SourcesColumns()
    {
        add(sourceId);
        add(type);
        add(host);
        add(path);
    }
};

class SourcesDialog : public Gtk::Dialog
{
 private:
    MainWindow* m_mainWindow;

    const SourcesColumns m_columns;

    Gtk::VBox m_vBox;
    Gtk::HBox m_hBox;
    Gtk::ScrolledWindow m_scrolledWindow;
    Gtk::TreeView m_treeView;
    Glib::RefPtr<Gtk::ListStore> m_refListStore;

    Gtk::Button m_addButton;
    Gtk::Button m_editButton;
    Gtk::Button m_deleteButton;

    void update();

 public:
    SourcesDialog(MainWindow* mainWindow);
    ~SourcesDialog();

    void open();
    void close(int responseId);
};

#endif
