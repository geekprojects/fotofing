#ifndef __FOTOFIND_INDEX_H_
#define __FOTOFIND_INDEX_H_

#include <vector>
#include <string>

#include "database.h"
#include "photo.h"

class Index
{
 private:
    Database* m_db;

 public:
    Index();
    ~Index();

    bool scanDirectory(std::string dir);

    bool saveTags(std::string pid, std::set<std::string> tags);
    std::set<std::string> getAllTags();
    std::set<std::string> getTags(std::string pid);

    std::vector<Photo*> getPhotos(std::vector<std::string> tags);
    std::vector<Photo*> getPhotos();
};

#endif
