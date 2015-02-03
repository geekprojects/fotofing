
#include <stdio.h>
#include <stdlib.h>

#include <fotofing/index.h>
#include <fotofing/utils.h>

#include "ui.h"
#include "mainwindow.h"

using namespace std;

int main(int argc, char** argv)
{
    FotofingUI app(argc, argv);

    string home = string(getenv("HOME"));
    string path = home + DEFAULT_DB_PATH;
    Index* index = new Index(path);
    index->open();

    MainWindow mainWindow(index);

    app.run(mainWindow);

    delete index;

    return 0;
}

