
#include <fotofing/index.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <string>

using namespace std;

static struct option longOptions[] =
{
    {"database", required_argument, 0, 'd'},
    {0, 0, 0, 0}
};

typedef void(*commandHandler_t)(Index*, int, char**);

void tags(Index* index, int argc, char** argv);
void photos(Index* index, int argc, char** argv);

struct command_t
{
    const char* command;
    commandHandler_t handler;
};

command_t g_commands[] =
{
    {"tags", tags},
    {"photos", photos},
    {NULL, NULL}
};

int main(int argc, char** argv)
{
    string database = string(getenv("HOME")) + DEFAULT_DB_PATH;

    while (true)
    {
        int optionIndex = 0;
        int c;

        c = getopt_long(argc, argv, "d:", longOptions, &optionIndex);
        if (c == -1)
        {
            break;
        }
        switch (c)
        {
            case 'd':
                database = string(optarg);
                break;
        }
    }

    printf("%s: database=%s\n", argv[0], database.c_str());

    if (optind >= argc)
    {
        printf("%s: Expected command\n", argv[0]);
        exit(-1);
    }

    Index* index = new Index(database);
    bool res;
    res = index->open();
    if (!res)
    {
        printf("%s: Failed to open database: %s\n", argv[0], database.c_str());
    }

    int i;
    bool found = false;
    const char* cmd = argv[optind];
    for (i = 0; g_commands[i].command != NULL; i++)
    {
        if (!strcmp(cmd, g_commands[i].command))
        {
            g_commands[i].handler(index, argc - optind, argv + optind);
            found = true;
            break;
        }
    }
    if (!found)
    {
        printf("%s: Unknown command %s\n", argv[0], cmd);
    }

    return 0;
}

void tags(Index* index, int argc, char** argv)
{
    if (argc < 2)
    {
        return;
    }

    if (!strcmp(argv[1], "list"))
    {
        set<string> tags = index->getAllTags();
        set<string>::iterator it;
        for (it = tags.begin(); it != tags.end(); it++)
        {
            printf("%s\n", it->c_str());
        }
    }
}

void photos(Index* index, int argc, char** argv)
{
    if (argc < 2)
    {
        return;
    }

    if (!strcmp(argv[1], "list"))
    {
        vector<Photo*> photos = index->getPhotos(NULL, NULL);
        vector<Photo*>::iterator it;
        for (it = photos.begin(); it != photos.end(); it++)
        {
            printf("%s\n", (*it)->getId().c_str());
        }
    }
    else if (!strcmp(argv[1], "search"))
    {
vector<string> tags;
int i;
for (i = 2; i < argc; i++)
{
tags.push_back(argv[i]);
}
        vector<Photo*> photos = index->getPhotos(tags, NULL, NULL);
        vector<Photo*>::iterator it;
        for (it = photos.begin(); it != photos.end(); it++)
        {
            printf("%s\n", (*it)->getId().c_str());
        }
    }
}


