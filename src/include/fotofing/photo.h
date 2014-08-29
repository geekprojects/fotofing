#ifndef __FOTOFING_PHOTO_H_
#define __FOTOFING_PHOTO_H_

#include <string>
#include <set>

#include <geek/gfx-surface.h>

class Photo
{
 private:
    std::string m_id;
    Geek::Gfx::Surface* m_thumbnail;
    time_t m_timestamp;
    std::set<std::string> m_tags;

 public:
    Photo(std::string id, Geek::Gfx::Surface* thumbnail, time_t timestamp);
    virtual ~Photo();

    std::string getId() { return m_id; }
    Geek::Gfx::Surface* getThumbnail() { return m_thumbnail; }
    time_t getTimestamp() { return m_timestamp; }

    void setTags(std::set<std::string>& tags) { m_tags = tags; }
    std::set<std::string> getTags() { return m_tags; }
};

#endif
