
#include <stdio.h>
#include <stdlib.h>

#include <fotofing/index.h>
#include <fotofing/utils.h>

#include "mainwindow.h"

using namespace std;

int main(int argc, char** argv)
{
    Glib::RefPtr<Gtk::Application> app;
    app = Gtk::Application::create(argc, argv, "com.geekprojects.fotofing");

    string home = string(getenv("HOME"));
    string path = home + DEFAULT_DB_PATH;
    Index* index = new Index(path);
    index->open();

    MainWindow mainWindow(index);

    app->run(mainWindow);

    delete index;

    return 0;
}

