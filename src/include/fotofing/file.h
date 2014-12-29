#ifndef __FOTOFING_FILE_H_
#define __FOTOFING_FILE_H_

#include <string>
#include <map>

#include <geek/gfx-surface.h>

struct TagData;

class File
{
 private:
    int64_t m_sourceId;
    std::string m_path;
    Geek::Gfx::Surface* m_image;
    Geek::Gfx::Surface* m_thumbnail;
    std::string m_fingerprint;

    Geek::Gfx::Surface* generateThumbnail(Geek::Gfx::Surface* image, int width, int height, bool fingerprint);

 public:
    File(int64_t sourceId, std::string path);
    ~File();

    int64_t getSourceId() { return m_sourceId; }
    std::string getPath() { return m_path; }

    bool scan();

    Geek::Gfx::Surface* getThumbnail() { return m_thumbnail; }
    std::string getFingerprint() { return m_fingerprint; }
    bool getTags(std::map<std::string, TagData*>& tags, time_t* timestamp);
};

#endif
