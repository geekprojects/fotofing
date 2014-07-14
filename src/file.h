#ifndef __FOTOFING_FILE_H_
#define __FOTOFING_FILE_H_

#include <string>
#include <set>

#include <beyond/surface.h>

class File
{
 private:
    std::string m_path;

 public:
    File(std::string path);
    ~File();

    std::string getPath() { return m_path; }

    Surface* generateThumbnail();
    bool getTags(std::set<std::string>& tags, time_t* timestamp);
};

#endif
