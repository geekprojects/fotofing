#ifndef __FOTOFING_FILE_H_
#define __FOTOFING_FILE_H_

#include <string>
#include <set>

#include <beyond/surface.h>

class File
{
 private:
    std::string m_path;
    Surface* m_thumbnail;
    std::string m_fingerprint;

 public:
    File(std::string path);
    ~File();

    std::string getPath() { return m_path; }

    bool scan();

    Surface* getThumbnail() { return m_thumbnail; }
    std::string getFingerprint() { return m_fingerprint; }
    bool getTags(std::set<std::string>& tags, time_t* timestamp);
};

#endif