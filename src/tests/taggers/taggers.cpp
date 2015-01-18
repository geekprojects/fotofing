
#include <dlfcn.h>

#include <fotofing/tagger.h>
#include <fotofing/utils.h>

using namespace std;

int main(int argc, char** argv)
{
#if 0
    if (argc < 2)
    {
        return 0;
    }


    void* taggerHandle = dlopen(argv[1], RTLD_LAZY);
printf("Tagger test: taggerHandle=%p\n", taggerHandle);

if (taggerHandle == NULL)
{
printf("Tagger test: Failed to open tagger: %s\n", dlerror());
return -1;
}

    TaggerInfo* taggerInfo = (TaggerInfo*)dlsym(taggerHandle, "g_fotofing_taggerInfo");
printf("Tagger test: taggerInfo=%p\n", taggerInfo);

printf("Tagger test: Tagger name: %s\n", taggerInfo->name);

Tagger* tagger = taggerInfo->newTagger();
printf("Tagger test: Tagger: %p\n", tagger);

delete tagger;

#endif

vector<TaggerInfo*> taggers = Tagger::findTaggers();
vector<TaggerInfo*>::iterator it;
for (it = taggers.begin(); it != taggers.end(); it++)
{
TaggerInfo* info = *it;
printf("Tagger test: %s: %s\n", info->path.c_str(), info->name);
}



   return 0;
}

