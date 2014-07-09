

#include "mainwindow.h"

using namespace std;

MainWindow::MainWindow(Index* index) :
    m_vBox(Gtk::ORIENTATION_VERTICAL)
{
    m_index = index;

    set_title("Fotofing");
    set_default_size(650, 400);

    //m_scrolledWindow.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    //m_scrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);


    m_model = Gtk::ListStore::create(m_columns);
    //m_model->set_sort_column(Gtk::TreeSortable::DEFAULT_SORT_COLUMN_ID, Gtk::SORT_ASCENDING);

    m_iconView.set_model(m_model);
    m_iconView.set_selection_mode(Gtk::SELECTION_MULTIPLE);

    m_iconView.set_text_column(m_columns.display_name);
    m_iconView.set_pixbuf_column(m_columns.pixbuf);

/*
    m_iconView.set_columns(-1);
    m_iconView.set_column_spacing(1);
    m_iconView.set_spacing(1);
    m_iconView.set_item_padding(1);
    m_iconView.set_margin(1);
    m_iconView.set_row_spacing(-1);
*/
    m_iconView.set_item_width(10);
    m_iconView.grab_focus();

    m_scrolledWindow.add(m_iconView);
    m_vBox.pack_start(m_scrolledWindow, Gtk::PACK_EXPAND_WIDGET);

    add(m_vBox);
    m_iconView.set_columns(-1);

    show_all();

    update();

}

MainWindow::~MainWindow()
{
}

void MainWindow::update()
{
    m_model->clear();

    vector<Photo> photos = m_index->getPhotos();
    vector<Photo>::iterator it;
    for (it = photos.begin(); it != photos.end(); it++)
    {
        Gtk::TreeModel::iterator iter = m_model->append();
        Gtk::TreeModel::Row row = *iter;

        Surface* thumbnail = it->getThumbnail();

        Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data(
            thumbnail->getData(),
            Gdk::COLORSPACE_RGB,
            true,
            8,
            thumbnail->getWidth(),
            thumbnail->getHeight(),
            thumbnail->getWidth() * 4
        );

        string displayName = it->getId().substr(0, 6) + "...";
        row[m_columns.display_name] = displayName;
        row[m_columns.pixbuf] = pixbuf;
    }
}

