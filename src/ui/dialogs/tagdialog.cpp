
#include "tagdialog.h"
#include "mainwindow.h"

using namespace std;

TagDialog::TagDialog(MainWindow* mainWindow) :
    Gtk::Dialog("Add Tag"),
    m_mainWindow(mainWindow),
    m_tagTextCombo(true)
{

    m_tagTypeCombo.append("Event");
    m_tagTypeCombo.append("Trip");
    m_tagTypeCombo.append("Location");
    m_tagTypeCombo.append("People");
    m_tagTypeCombo.append("Other");
    m_tagTypeCombo.set_active(1);
    m_tagTypeCombo.signal_changed().connect(sigc::mem_fun(
        *this,
        &TagDialog::onTypeChanged));

    m_tagTextCombo.get_entry()->signal_changed().connect(sigc::mem_fun(
        *this,
        &TagDialog::onTextChanged));

    m_vBox.pack_start(m_tagTypeCombo, Gtk::PACK_SHRINK);
    m_vBox.pack_start(m_tagTextCombo, Gtk::PACK_SHRINK);
    m_vBox.pack_start(m_tagDisplay, Gtk::PACK_SHRINK);
    get_content_area()->pack_start(m_vBox, Gtk::PACK_SHRINK);

    add_button("_Add", Gtk::RESPONSE_OK);
    add_button("_Cancel", Gtk::RESPONSE_CANCEL);

    show_all_children();

    // Make sure the combobox has the right entries
    onTypeChanged();
}

TagDialog::~TagDialog()
{
}

void TagDialog::onTypeChanged()
{
    Glib::ustring type = m_tagTypeCombo.get_active_text();

    m_tagTextCombo.remove_all();

    set<string> children;
    if (type != "Other")
    {
        children = m_mainWindow->getIndex()->getChildTags(type);
    }

    set<string>::iterator it;
    for (it = children.begin(); it != children.end(); it++)
    {
        // Strip the current type off the beginning
        string tag = (*it).substr(type.length() + 1);
        m_tagTextCombo.append(tag);
    }

    updateTag();
}

void TagDialog::onTextChanged()
{
    updateTag();
}

void TagDialog::updateTag()
{
    m_tagDisplay.set_label(getTag());
}

string TagDialog::getTag()
{
    string tag = "";
    Glib::ustring type = m_tagTypeCombo.get_active_text();
    Glib::ustring text = m_tagTextCombo.get_active_text();
    if (type != "Other")
    {
        tag = type + "/" + text;
    }
    else
    {
        tag = text;
    }
    return tag;
}

