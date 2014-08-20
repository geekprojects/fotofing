
#include "uiutils.h"

using namespace std;

bool UIUtils::confirm(Gtk::Window& parent, string title, string text)
{
    Gtk::MessageDialog dialog(
        parent,
        title,
        false,
        Gtk::MESSAGE_QUESTION,
        Gtk::BUTTONS_OK_CANCEL);
    dialog.set_secondary_text(text);

    int result = dialog.run();

    return (result == Gtk::RESPONSE_OK);
}

bool UIUtils::promptString(
    Gtk::Window& parent,
    string title,
    string text,
    string def,
    string& result)
{
    UIUtils::PromptStringDialog dialog(
        parent,
        title,
        text,
        def);

    int res = dialog.run();

    string str = def;

    if (res == Gtk::RESPONSE_OK)
    {
        result = dialog.getString();
        return true;
    }
    return false;
}

UIUtils::PromptStringDialog::PromptStringDialog(
    Gtk::Window& parent,
    string title,
    string text,
    string def) :
    Gtk::Dialog(title),
    m_label(text)
{
    m_entry.set_text(def);

    m_vBox.pack_start(m_label);
    m_vBox.pack_start(m_entry);
    get_content_area()->pack_start( m_vBox, Gtk::PACK_SHRINK);

    add_button("_Ok", Gtk::RESPONSE_OK);
    add_button("_Cancel", Gtk::RESPONSE_CANCEL);

    show_all_children();
}

UIUtils::PromptStringDialog::~PromptStringDialog()
{
}

