#ifndef __FOTOFING_SOURCE_H_
#define __FOTOFING_SOURCE_H_

#include <string>
#include <vector>

#include <stdint.h>

#include <fotofing/file.h>
#include <fotofing/utils.h>

class Index;

class Source
{
 protected:
    int64_t m_sourceId;
    std::string m_type;
    std::string m_host;
    std::string m_path;

 public:
    Source(const Source& s);
    Source(
        int64_t sourceId,
        std::string type,
        std::string host,
        std::string path);
    virtual ~Source();

    int64_t getSourceId() { return m_sourceId; }
    std::string getType() { return m_type; }
    std::string getHost() { return m_host; }
    std::string getPath() { return m_path; }

    void setSourceId(int64_t sourceId) { m_sourceId = sourceId; }

    virtual bool scan(Index* index);
};

class FileSource : public Source
{
 protected:
    bool scanDirectory(std::string path, Index* index);

 public:
    FileSource(const Source& s);
    FileSource(
        int64_t sourceId,
        std::string host,
        std::string path);
    virtual ~FileSource();

    virtual bool scan(Index* index);
};

#endif
