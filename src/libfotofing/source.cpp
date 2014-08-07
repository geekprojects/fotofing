
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fotofing/source.h>
#include <fotofing/index.h>
#include <fotofing/utils.h>

using namespace std;

Source::Source()
{
    m_sourceId = 0;
    m_type = "";
    m_host = "";
    m_path = "";
}

Source::Source(const Source& s)
{
    m_sourceId = s.m_sourceId;
    m_type = s.m_type;
    m_host = s.m_host;
    m_path = s.m_path;
}

Source::Source(int64_t sourceId, string type, string host, string path)
{
    m_sourceId = sourceId;
    m_type = type;
    m_host = host;
    m_path = path;
}

Source::~Source()
{
}

bool Source::scan(Index* index)
{
    return true;
}

FileSource::FileSource(const Source& s) :
    Source(s)
{
    if (m_type != "Source")
    {
        printf("FileSource::FileSource: ERROR: Source source is not a File!\n");
    }
}

FileSource::FileSource(int64_t sourceId, string host, string path) :
    Source(sourceId, "File", host, path)
{
}

FileSource::~FileSource()
{
}

bool FileSource::scan(Index* index)
{
    string hostname = getHostName();
    if (m_host != hostname)
    {
        printf("FileSource::getFiles: WARNING: Wrong host\n");
        return false;
    }
    return scanDirectory(m_path, index);
}

bool FileSource::scanDirectory(string dir, Index* index)
{
    DIR* fd = opendir(dir.c_str());
    if (fd == NULL)
    {
        return false;
    }

    while (true)
    {
        dirent* dirent = readdir(fd);
        if (dirent == NULL)
        {
            break;
        }

        if (dirent->d_name[0] == '.')
        {
            continue;
        }

        struct stat stat;
        string path = dir + "/" + dirent->d_name;
        lstat(path.c_str(), &stat);

        if (S_ISDIR(stat.st_mode))
        {
            scanDirectory(path, index);
        }
        else if (S_ISREG(stat.st_mode))
        {
            string ext = "";
            size_t extpos = path.rfind('.');
            if (extpos != path.npos)
            {
                ext = path.substr(extpos);
            }

            // TODO: Replace this with proper file type recognition
            if (!(ext == ".JPG" || ext == ".jpg"))
            {
                continue;
            }
            File file(path);
            index->scanFile(this, &file);
        }
    }
    closedir(fd);

    return true;
}

