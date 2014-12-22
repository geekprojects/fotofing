
#include <set>
#include <string>

#include "photodetails.h"
#include "mainwindow.h"
#include "uiutils.h"

using namespace std;

PhotoDetails::PhotoDetails(Library* library) :
    Gtk::Paned(Gtk::ORIENTATION_VERTICAL)
{
    m_library = library;
    m_photo = NULL;

    // Photo properties
    m_photoPropListStore = Gtk::ListStore::create(m_photoPropColumns);
    m_photoPropTreeView.set_model(m_photoPropListStore);
    m_photoPropTreeView.append_column_editable("Property", m_photoPropColumns.property);
    m_photoPropTreeView.append_column_editable("Value", m_photoPropColumns.value);

    m_photoPropScrollWindow.add(m_photoPropTreeView);
    m_photoPropFrame.set_shadow_type(Gtk::SHADOW_IN);
    m_photoPropFrame.add(m_photoPropScrollWindow);
    pack1(m_photoPropFrame, true, false);

    // Photo tags
    m_tagView.signal_delete_tags().connect(sigc::mem_fun(
        *this,
        &PhotoDetails::onDeleteTags));
    m_photoTagFrame.set_shadow_type(Gtk::SHADOW_IN);
    m_photoTagFrame.add(m_tagView);
    pack2(m_photoTagFrame, true, false);
}

PhotoDetails::~PhotoDetails()
{
}

void PhotoDetails::displayDetails(Photo* photo)
{
    m_photo = photo;

    m_photoPropListStore->clear();
    Gtk::TreeRow propRow;
    propRow = *(m_photoPropListStore->append());
    propRow[m_photoPropColumns.property] = "Id";
    propRow[m_photoPropColumns.value] = photo->getId();

    time_t ts = photo->getTimestamp();
    struct tm tm;
    localtime_r(&ts, &tm);

    char tsbuf[64];
    strftime(tsbuf, 64, "%Ex", &tm);
    propRow = *(m_photoPropListStore->append());
    propRow[m_photoPropColumns.property] = "Date";
    propRow[m_photoPropColumns.value] = string(tsbuf);

    strftime(tsbuf, 64, "%X", &tm);
    propRow = *(m_photoPropListStore->append());
    propRow[m_photoPropColumns.property] = "Time";
    propRow[m_photoPropColumns.value] = string(tsbuf);

    // Tags
    set<string> tags = photo->getTags();
    if (tags.size() == 0)
    {
        // Only do this if we haven't already retrieved the tags
        tags = m_library->getIndex()->getTags(photo->getId());
        photo->setTags(tags);
    }

    m_tagView.update(tags);
}

void PhotoDetails::updateTags()
{
    if (m_photo == NULL)
    {
        return;
    }

    // Refresh the photo's tags
    set<string> updatedTags;
    updatedTags = m_library->getIndex()->getTags(m_photo->getId());
    m_photo->setTags(updatedTags);
    m_tagView.update(updatedTags);

}

void PhotoDetails::onDeleteTags(vector<Tag*> tags)
{
    if (m_photo == NULL)
    {
        return;
    }

    int res;
    res = UIUtils::confirm(
        *m_library->getMainWindow(),
        "Remove selected tags?",
        "Are you sure you wish to remove the selected tags?");
    if (res)
    {
        vector<Tag*>::iterator it;
        for (it = tags.begin(); it != tags.end(); it++)
        {
            Tag* tag = *it;
            printf(
                "MainWindow::onDeleteTags: Deleting tag: %s\n",
                tag->getTagName().c_str());
            m_library->getIndex()->removeTag(m_photo->getId(), tag->getTagName());
        }

        updateTags();

        // Just in case this we just removed the last instance of a tags
        m_library->updateTags();
    }
}

