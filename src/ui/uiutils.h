#ifndef __FOTOFING_UI_UIUTILS_H_
#define __FOTOFING_UI_UIUTILS_H_

#include <gtkmm.h>
#include <string.h>

namespace UIUtils
{

    class PromptStringDialog : public Gtk::Dialog
    {
     private:

        Gtk::VBox m_vBox;
        Gtk::Label m_label;
        Gtk::Entry m_entry;

     public:
        PromptStringDialog(
            Gtk::Window& parent,
            std::string title,
            std::string text,
            std::string def);
        ~PromptStringDialog();

        std::string getString() { return m_entry.get_text(); }
    };

    bool confirm(Gtk::Window& parent, std::string title, std::string text);
    bool promptString(
        Gtk::Window& parent,
        std::string title,
        std::string text,
        std::string def,
        std::string& result);
};

#endif
