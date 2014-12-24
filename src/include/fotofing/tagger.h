#ifndef __FOTOFING_TAGGER_H_
#define __FOTOFING_TAGGER_H_

#include <string>
#include <vector>

#include <geek/gfx-surface.h>

#include <fotofing/photo.h>

class Tagger;

typedef Tagger*(*newTagger_t)();

struct TaggerInfo
{
    const char* name;
    newTagger_t newTagger;

    // Filled in by fotofing
    std::string path;
    void* handle;
};

class Tagger
{
 private:

 public:
    Tagger();
    virtual ~Tagger();

    virtual void tag(std::string path, Geek::Gfx::Surface* image);

    static std::vector<TaggerInfo*> findTaggers();
};

#define DECLARE_TAGGER(_name, _class) \
    static Tagger* newTagger() \
    { \
        return new _class(); \
    } \
    TaggerInfo g_fotofing_taggerInfo = \
    { \
        .name = _name, \
        .newTagger = newTagger \
    }

#endif
