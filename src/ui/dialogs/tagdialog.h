#ifndef __FOFOFING_UI_TAGDIALOG_H_
#define __FOFOFING_UI_TAGDIALOG_H_

#include <gtkmm.h>

class MainWindow;

class TagDialog : public Gtk::Dialog
{
 private:
    MainWindow* m_mainWindow;

    Gtk::VBox m_vBox;
    Gtk::ComboBoxText m_tagTypeCombo;
    Gtk::ComboBoxText m_tagTextCombo;
    Gtk::Label m_tagDisplay;

    void onTypeChanged();
    void onTextChanged();

    void updateTag();

 public:
    TagDialog(MainWindow* mainWindow);
    ~TagDialog();

    std::string getTag();
};

#endif
