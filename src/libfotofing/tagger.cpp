
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>

#include <fotofing/tagger.h>

using namespace std;

static vector<TaggerInfo*> g_taggers;

Tagger::Tagger()
{
}

Tagger::~Tagger()
{
}

bool Tagger::tag(string path, Geek::Gfx::Surface* image, std::map<std::string, TagData*>& tags)
{
    return false;
}

const char* taggerPaths[] = {
    "src/taggers/exif/.libs/",
    "src/taggers/histogram/.libs/",
    "/usr/lib",
    "/usr/local/lib",
    NULL
};

#define TAGGER_LIB_PREFIX "libfotofing-tagger-"

int taggerfilter(const struct dirent* e)
{
    string name = string(e->d_name);
    return (
        name.find(TAGGER_LIB_PREFIX) == 0 &&
        name.rfind(".so") == name.size() - 3 );
}

vector<TaggerInfo*> Tagger::findTaggers()
{
    if (g_taggers.size() > 0)
    {
        return g_taggers;
    }

    int i;
    for (i = 0; taggerPaths[i] != NULL; i++)
    {
        const char* path = taggerPaths[i];
        printf("Tagger::findTaggers: Checking %s\n", path);

        struct dirent** namelist;
        int n;
        n = scandir(path, &namelist, taggerfilter, alphasort);
        if (n < 0)
        {
            printf("Tagger::findTaggers: Failed to scan directory\n");
            continue;
        }

        while (n--)
        {
            string taggerlib = string(path) + "/" + string(namelist[n]->d_name);
            printf("%s\n", taggerlib.c_str());

            void* taggerHandle = dlopen(taggerlib.c_str(), RTLD_LAZY);

            if (taggerHandle == NULL)
            {
                printf("Tagger test: Failed to open tagger: %s\n", dlerror());
                continue;
            }

            TaggerInfo* taggerInfo = (TaggerInfo*)dlsym(taggerHandle, "g_fotofing_taggerInfo");

            taggerInfo->path = taggerlib;
            taggerInfo->handle = taggerHandle;
            g_taggers.push_back(taggerInfo);

            free(namelist[n]);
        }
        free(namelist);
    }

return g_taggers;
}

