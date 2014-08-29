#ifndef __FOTOFING_FILE_H_
#define __FOTOFING_FILE_H_

#include <string>
#include <set>

#include <geek/gfx-surface.h>

class File
{
 private:
    int64_t m_sourceId;
    std::string m_path;
    Geek::Gfx::Surface* m_thumbnail;
    std::string m_fingerprint;

 public:
    File(int64_t sourceId, std::string path);
    ~File();

    int64_t getSourceId() { return m_sourceId; }
    std::string getPath() { return m_path; }

    bool scan();

    Geek::Gfx::Surface* getThumbnail() { return m_thumbnail; }
    std::string getFingerprint() { return m_fingerprint; }
    bool getTags(std::set<std::string>& tags, time_t* timestamp);
};

#endif
