#ifndef __FOFOFING_UI_TAGDIALOG_H_
#define __FOFOFING_UI_TAGDIALOG_H_

#include <gtkmm.h>

class TagDialog : public Gtk::Dialog
{
 private:
    Gtk::VBox m_vBox;
    Gtk::ComboBoxText m_tagTypeCombo;
    Gtk::ComboBoxText m_tagTextCombo;
    Gtk::Label m_tagDisplay;

    void onTypeChanged();
    void onTextChanged();

    void updateTag();

 public:
    TagDialog();
    ~TagDialog();

    std::string getTag();
};

#endif
