
#include <set>
#include <string>

#include "photodetails.h"
#include "mainwindow.h"

using namespace std;

PhotoDetails::PhotoDetails(MainWindow* mainWindow) :
    Gtk::Paned(Gtk::ORIENTATION_VERTICAL)
{
    m_mainWindow = mainWindow;

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
    m_photoTagListStore = Gtk::ListStore::create(m_photoTagColumns);
    m_photoTagTreeView.set_model(m_photoTagListStore);
    m_photoTagScrollWindow.add(m_photoTagTreeView);
    m_photoTagTreeView.append_column_editable("Tag", m_photoTagColumns.tag);

    m_photoTagFrame.set_shadow_type(Gtk::SHADOW_IN);
    m_photoTagFrame.add(m_photoTagScrollWindow);
    pack2(m_photoTagFrame, true, false);
}

PhotoDetails::~PhotoDetails()
{
}

void PhotoDetails::displayDetails(Photo* photo)
{
    m_photoPropListStore->clear();
    Gtk::TreeRow propRow;
    propRow = *(m_photoPropListStore->append());
    propRow[m_photoPropColumns.property] = "Id";
    propRow[m_photoPropColumns.value] = photo->getId();

    time_t ts = photo->getTimestamp();
    struct tm tm;
    localtime_r(&ts, &tm);

    char tsbuf[64];
    strftime(tsbuf, 64, "%x", &tm);
    propRow = *(m_photoPropListStore->append());
    propRow[m_photoPropColumns.property] = "Date";
    propRow[m_photoPropColumns.value] = string(tsbuf);

    strftime(tsbuf, 64, "%X", &tm);
    propRow = *(m_photoPropListStore->append());
    propRow[m_photoPropColumns.property] = "Time";
    propRow[m_photoPropColumns.value] = string(tsbuf);

    // Tags
    m_photoTagListStore->clear();
    set<string> tags = photo->getTags();
    if (tags.size() == 0)
    {
        // Only do this if we haven't already retrieved the tags
        tags = m_mainWindow->getIndex()->getTags(photo->getId());
        photo->setTags(tags);
    }

    set<string>::iterator tagIt;
    for (tagIt = tags.begin(); tagIt != tags.end(); tagIt++)
    {
        Gtk::TreeRow tagRow;
        tagRow = *(m_photoTagListStore->append());
        tagRow[m_photoTagColumns.tag] = *tagIt;
    }
}
