#ifndef __FOTOFIND_INDEX_H_
#define __FOTOFIND_INDEX_H_

#include <vector>

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

    std::vector<Photo> getPhotos();
};

#endif
