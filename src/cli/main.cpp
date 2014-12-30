
#include <fotofing/index.h>
#include <fotofing/utils.h>

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
void sources(Index* index, int argc, char** argv);

struct command_t
{
    const char* command;
    commandHandler_t handler;
};

command_t g_commands[] =
{
    {"tags", tags},
    {"photos", photos},
    {"sources", sources},
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
    delete index;

    return 0;
}

static void printTags(set<string> tags)
{
    set<string>::iterator it;
    for (it = tags.begin(); it != tags.end(); it++)
    {
        printf("%s\n", it->c_str());
    }
}

void tags(Index* index, int argc, char** argv)
{
    if (argc < 2)
    {
        return;
    }

    if (!strcmp(argv[1], "list"))
    {
        printTags(index->getAllTags());
    }
    else if (!strcmp(argv[1], "children"))
    {
        if (argc < 3)
        {
            return;
        }
        printTags(index->getChildTags(argv[2]));
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
    else if (!strcmp(argv[1], "info"))
    {
        if (argc < 3)
        {
            return;
        }
        Photo* photo = index->getPhoto(argv[2]);
        printf("PID: %s\n", photo->getId().c_str());
        printf(
            "Timestamp: %s\n",
            timeToString(photo->getTimestamp(), true).c_str());
    }
    else if (!strcmp(argv[1], "tags"))
    {
        if (argc < 3)
        {
            return;
        }
        printTags(index->getTags(argv[2]));
    }
    else if (!strcmp(argv[1], "files"))
    {
        if (argc < 3)
        {
            return;
        }
        vector<File*> files = index->getFiles(argv[2]);
        vector<File*>::iterator it;
        for (it = files.begin(); it != files.end(); it++)
        {
            File* f = *it;
            printf("%ld %s\n", f->getSourceId(), f->getPath().c_str());
        }
    }
}

void sources(Index* index, int argc, char** argv)
{
    if (argc < 2)
    {
        return;
    }

    if (!strcmp(argv[1], "list"))
    {
        vector<Source*> sources = index->getSources();
        vector<Source*>::iterator it;

        for (it = sources.begin(); it != sources.end(); it++)
        {
            Source* s = *it;
            printf(
                "%4ld %-6s %-10s %s\n",
                s->getSourceId(),
                s->getType().c_str(),
                s->getHost().c_str(),
                s->getPath().c_str());
        }
    }
    else if (!strcmp(argv[1], "add"))
    {
        if (argc < 4)
        {
            printf("Not enough arguments\n");
            return;
        }

        if (strcmp(argv[2], "file") != 0)
        {
            printf("Source type must be \"file\"\n");
            return;
        }

        const char* path = argv[3];
        FileSource fileSource(0, getHostName(), path);

        index->addSource(&fileSource);
        index->scanSource(&fileSource, NULL);
    }
}

