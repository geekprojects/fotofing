#ifndef __FOTOFIND_INDEX_H_
#define __FOTOFIND_INDEX_H_

#include <vector>
#include <string>

#include <time.h>

#include "database.h"
#include "photo.h"
#include "source.h"
#include "file.h"

class Index
{
 private:
    Database* m_db;

    bool saveSource(Source* s);

 public:
    Index();
    ~Index();

    bool saveTags(std::string pid, std::set<std::string> tags);
    std::set<std::string> getAllTags();
    std::set<std::string> getTags(std::string pid);

    // Remove a tag from a photo
    bool removeTag(std::string pid, std::string tag);

    // Remove all occurences of a tag
    bool removeTag(std::string tag);

    std::vector<Photo*> getPhotos(std::vector<std::string> tags, time_t* from, time_t* to);
    std::vector<Photo*> getPhotos(time_t* from, time_t* to);

    std::vector<File*> getFiles(std::string pid);

    bool addFileSource(std::string path);
    bool scanFile(Source* source, File* f);
    bool scanSource(Source* s);
    std::vector<Source> getSources();
};

#endif
