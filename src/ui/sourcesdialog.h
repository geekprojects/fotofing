#ifndef __FOTOFING_UI_SOURCESDIALOG_H_
#define __FOTOFING_UI_SOURCESDIALOG_H_

#include <gtkmm.h>

#include <fotofing/source.h>

class MainWindow;

struct SourcesColumns : public Gtk::TreeModelColumnRecord
{
    Gtk::TreeModelColumn<int64_t> sourceId;
    Gtk::TreeModelColumn<Glib::ustring> type;
    Gtk::TreeModelColumn<Glib::ustring> host;
    Gtk::TreeModelColumn<Glib::ustring> path;
    Gtk::TreeModelColumn<Source> source;

    SourcesColumns()
    {
        add(sourceId);
        add(type);
        add(host);
        add(path);
        add(source);
    }
};

class SourceDialog : public Gtk::Dialog
{
 private:
    Gtk::Grid m_grid;
    Gtk::Label m_typeLabel;
    Gtk::ComboBoxText m_typeComboBox;
    Gtk::Label m_hostLabel;
    Gtk::Entry m_hostEdit;

    Gtk::Label m_pathLabel;
    Gtk::HBox m_pathBox;
    Gtk::Entry m_pathEdit;
    Gtk::Button m_pathButton;

 public:
    SourceDialog(Gtk::Window& parent, std::string host, std::string path);
    virtual ~SourceDialog();

    void onPathButton();

    std::string getType();
    std::string getHost();
    std::string getPath();
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
    Glib::RefPtr<Gtk::TreeSelection> m_refTreeSelection;

    Gtk::Button m_addButton;
    Gtk::Button m_deleteButton;

    void update();

    bool getSelectedSource(Source& source);

 public:
    SourcesDialog(MainWindow* mainWindow);
    ~SourcesDialog();

    void open();
    void close(int responseId);

    void onAddClicked();
    void onDeleteClicked();
};

#endif
