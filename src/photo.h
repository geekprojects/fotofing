#ifndef __FOTOFING_PHOTO_H_
#define __FOTOFING_PHOTO_H_

#include <string>

#include <beyond/surface.h>

class Photo
{
 private:
    std::string m_id;
    Surface* m_thumbnail;

 public:
    Photo(std::string id, Surface* thumbnail);
    virtual ~Photo();

    std::string getId() { return m_id; }
    Surface* getThumbnail() { return m_thumbnail; }
};

#endif
