
#include <stdio.h>
#include <stdlib.h>

#include "index.h"
#include "mainwindow.h"

using namespace std;

int main(int argc, char** argv)
{
    Glib::RefPtr<Gtk::Application> app;
    app = Gtk::Application::create(argc, argv, "com.geekprojects.fotofing");


    Index* index = new Index();
    //index->scanDirectory("/data/home/ian/Pictures/2014/07/05");
    //index->scanDirectory("/data/home/ian/Pictures/flickr/Syria and Lebanon");
    index->scanDirectory("test");

    MainWindow mainWindow(index);

    app->run(mainWindow);

    delete index;

    //scanDirectory("/data/home/ian/Pictures/2014");
    return 0;
}

