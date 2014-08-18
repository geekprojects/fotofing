#ifndef __FOTOFIND_INDEX_H_
#define __FOTOFIND_INDEX_H_

#include <vector>
#include <string>

#include <time.h>

#include "database.h"
#include "photo.h"
#include "source.h"
#include "file.h"

#define DEFAULT_DB_PATH "/.fotofing/fotofing.db"

class Index;

class IndexClient
{
 protected:
    Index* m_index;

 public:
    virtual ~IndexClient() {}

    virtual void scanProgress(
        Source* source,
        int complete,
        int total,
        std::string info)
    {
    }
};

class Index
{
 private:
    std::string m_path;
    Database* m_db;


 public:
    Index(std::string path);
    ~Index();

    bool open();

    bool saveTags(std::string pid, std::set<std::string> tags);
    std::set<std::string> getAllTags();
    std::set<std::string> getTags(std::string pid);
    std::set<std::string> getChildTags(std::string tag);

    // Remove a tag from a photo
    bool removeTag(std::string pid, std::string tag);

    // Remove all occurences of a tag
    bool removeTag(std::string tag);

    std::vector<Photo*> getPhotos(std::vector<std::string> tags, time_t* from, time_t* to);
    std::vector<Photo*> getPhotos(time_t* from, time_t* to);
    Photo* getPhoto(std::string id);

    std::vector<File*> getFiles(std::string pid);

    bool addFileSource(std::string path);
    bool addSource(Source* s);
    bool scanFile(Source* source, File* f);
    bool scanSource(Source* s, IndexClient* client = NULL);
    bool scanSources(IndexClient* client = NULL);
    bool removeSource(Source* s);
    std::vector<Source*> getSources();
};

#endif
