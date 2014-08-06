
#include "photoview.h"
#include "mainwindow.h"

#include <sys/wait.h>

using namespace std;

PhotoView::PhotoView(MainWindow* mainWindow)
{
    m_mainWindow = mainWindow;

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
        m_photos = m_mainWindow->getIndex()->getPhotos(tagStrings, &from, &to);
    }
    else
    {
        m_photos = m_mainWindow->getIndex()->getPhotos(&from, &to);
    }

    vector<Photo*>::iterator it;
    for (it = m_photos.begin(); it != m_photos.end(); it++)
    {
        Gtk::TreeModel::iterator iter = m_model->append();
        Gtk::TreeModel::Row row = *iter;

        Photo* photo = *it;
        Surface* thumbnail = photo->getThumbnail();

        Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data(
            thumbnail->getData(),
            Gdk::COLORSPACE_RGB,
            true,
            8,
            thumbnail->getWidth(),
            thumbnail->getHeight(),
            thumbnail->getWidth() * 4
        );

        string displayName = photo->getId().substr(0, 6) + "...";
        row[m_photoColumns.display_name] = displayName;
        row[m_photoColumns.pixbuf] = pixbuf;
        //row[m_photoColumns.photo] = Glib::RefPtr<Photo>(photo);
        row[m_photoColumns.photo] = photo;
        row[m_photoColumns.timestamp] = photo->getTimestamp();

        //delete photo;
    }

}

void PhotoView::onIconViewItemActivated(const Gtk::TreeModel::Path& path)
{
    Photo* photo = getPhotoFromPath(path);
    m_mainWindow->displayDetails(photo);

    vector<File*> files = m_mainWindow->getIndex()->getFiles(photo->getId());
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
        m_mainWindow->displayDetails(getPhotoFromPath(selected.at(0)));
    }
}

int PhotoView::onIconViewSort(const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b)
{
    Gtk::TreeModel::Row row_a = *a;
    Gtk::TreeModel::Row row_b = *b;

    return row_a[m_photoColumns.timestamp] - row_b[m_photoColumns.timestamp];
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

