
#include "photoview.h"
#include "mainwindow.h"
#include "uiutils.h"

#include <sys/wait.h>

using namespace std;
using namespace Geek::Gfx;

PhotoView::PhotoView(Library* library)
{
    m_library = library;

    m_model = Gtk::ListStore::create(m_photoColumns);
    m_model->set_default_sort_func(sigc::mem_fun(
        *this,
        &PhotoView::onIconViewSort));
    m_model->set_sort_column(
        Gtk::TreeSortable::DEFAULT_SORT_COLUMN_ID,
        Gtk::SORT_ASCENDING);

    m_iconView.set_model(m_model);
    m_iconView.set_selection_mode(Gtk::SELECTION_MULTIPLE);

    m_iconView.set_text_column(m_photoColumns.display_name);
    m_iconView.set_pixbuf_column(m_photoColumns.pixbuf);

    m_iconView.set_item_width(10);
    m_iconView.grab_focus();

    m_iconView.set_activate_on_single_click(false);
    m_iconView.signal_item_activated().connect(sigc::mem_fun(
        *this,
        &PhotoView::onIconViewItemActivated));
    m_iconView.signal_selection_changed().connect(sigc::mem_fun(
        *this,
        &PhotoView::onIconViewSelectionChanged));

    // Add context menu
    Gtk::MenuItem* item = Gtk::manage(new Gtk::MenuItem("_Add Tag", true));
    item->signal_activate().connect(sigc::mem_fun(
        *this,
        &PhotoView::addTag));
    m_popupMenu.append(*item);
    item = Gtk::manage(new Gtk::MenuItem("_Rename", true));
    item->signal_activate().connect(sigc::mem_fun(
        *this,
        &PhotoView::rename));
    m_popupMenu.append(*item);

    m_popupMenu.accelerate(m_iconView);
    m_popupMenu.show_all();

    m_iconView.signal_button_press_event().connect(sigc::mem_fun(
        *this,
        &PhotoView::onButtonPress), false);

    add(m_iconView);
}

PhotoView::~PhotoView()
{
    freePhotos();
}

void PhotoView::update(std::vector<Tag*> tags, time_t from, time_t to)
{
    m_model->clear();

    freePhotos();

    if (tags.size() > 0)
    {
        vector<string> tagStrings;
        vector<Tag*>::iterator it;
        for (it = tags.begin(); it != tags.end(); it++)
        {
            tagStrings.push_back((*it)->getTagName());
        }
        m_photos = m_library->getIndex()->getPhotos(tagStrings, &from, &to);
    }
    else
    {
        m_photos = m_library->getIndex()->getPhotos(&from, &to);
    }

    vector<Photo*>::iterator it;
    for (it = m_photos.begin(); it != m_photos.end(); it++)
    {

        Photo* photo = *it;
        Surface* thumbnail = photo->getThumbnail();

        Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data(
            thumbnail->getData(),
            Gdk::COLORSPACE_RGB,
            true,
            8,
            thumbnail->getWidth(),
            thumbnail->getHeight() - 1,
            thumbnail->getWidth() * 4);

        string title = m_library->getIndex()->getProperty(
            photo->getId(),
            "Title");

        string displayName = title;
        if (displayName == "")
        {
            displayName = photo->getId().substr(0, 6) + "...";
        }

        Gtk::TreeModel::iterator iter = m_model->append();
        Gtk::TreeModel::Row row = *iter;
        row[m_photoColumns.display_name] = displayName;
        row[m_photoColumns.pixbuf] = pixbuf;
        row[m_photoColumns.photo] = photo;
    }
}

void PhotoView::selectAll()
{
    m_iconView.select_all();
}

void PhotoView::onIconViewItemActivated(const Gtk::TreeModel::Path& path)
{
    Photo* photo = getPhotoFromPath(path);
    m_library->displayDetails(photo);

    vector<File*> files = m_library->getIndex()->getFiles(photo->getId());
    if (files.size() > 0)
    {
        pid_t childPid = fork();
        if (childPid == 0)
        {
            const char* argv[] =
            {
                "/usr/bin/qiv",
                "-fm",
                files.at(0)->getPath().c_str(),
                (const char*)NULL
            };
            execvp(argv[0], (char**)argv);
            printf("PhotoView::onIconViewItemActivated: Failed to show image\n");

            exit(0);
        }
        else
        {
            pid_t tpid;
            do
            {
                int status;
                tpid = wait(&status);
            }
            while (tpid != childPid);
        }
    }
}

void PhotoView::onIconViewSelectionChanged()
{
    vector<Gtk::TreePath> selected = m_iconView.get_selected_items();
    if (selected.size() > 0)
    {
        m_library->displayDetails(getPhotoFromPath(selected.at(0)));
    }
}

int PhotoView::onIconViewSort(const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b)
{
    Gtk::TreeModel::Row row_a = *a;
    Gtk::TreeModel::Row row_b = *b;
    Photo* photoa = (row_a[m_photoColumns.photo]);
    Photo* photob = (row_b[m_photoColumns.photo]);

    if (photoa == NULL)
    {
        return true;
    }
    else if (photob == NULL)
    {
        return false;
    }

    return photoa->getTimestamp() - photob->getTimestamp();
}

bool PhotoView::onButtonPress(GdkEventButton* event)
{
    bool res = false;

    //res = m_iconView.on_button_press_event(event);

    if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
    {
        m_popupMenu.popup(event->button, event->time);
    }

    return res;
}

Photo* PhotoView::getPhotoFromPath(Gtk::TreePath path)
{
    Gtk::TreeModel::iterator iter = m_model->get_iter(path);
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        return row[m_photoColumns.photo];
    }
    return NULL;
}

void PhotoView::freePhotos()
{
    vector<Photo*>::iterator it;
    for (it = m_photos.begin(); it != m_photos.end(); it++)
    {
        delete *it;
    }
    m_photos.clear();
}

vector<Photo*> PhotoView::getSelectedPhotos()
{
    vector<Photo*> photos;
    vector<Gtk::TreePath> selected = m_iconView.get_selected_items();
    vector<Gtk::TreePath>::iterator it;

    for (it = selected.begin(); it != selected.end(); it++)
    {
        photos.push_back(getPhotoFromPath(*it));
    }

    return photos;
}

void PhotoView::addTag()
{
}

void PhotoView::rename()
{
    vector<Photo*> photos = getSelectedPhotos();
    if (photos.size() == 1)
    {
        Photo* photo = photos.at(0);

        string title = m_library->getIndex()->getProperty(
            photo->getId(),
            "Title");

        bool res;
        res = UIUtils::promptString(
            *m_library->getMainWindow(),
            "Photo Title",
            "Please give this photo a title",
            title,
            title);
        if (res)
        {
            m_library->getIndex()->setProperty(
                photo->getId(),
                "Title",
                title);
            m_library->update();
        }
    }
}

