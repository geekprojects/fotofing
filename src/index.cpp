
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fingerprint.h"
#include "index.h"
#include "file.h"

using namespace std;

Index::Index()
{
    vector<Table> schema;

    Table photosTable;
    photosTable.name = "photos";
    photosTable.columns.insert(Column("pid", true)); // Thumbnail
    photosTable.columns.insert(Column("thumbnail")); // Image Blob
    photosTable.columns.insert(Column("timestamp")); // Timestamp
    schema.push_back(photosTable);

    Table filesTable;
    filesTable.name = "files";
    filesTable.columns.insert(Column("path", true));
    filesTable.columns.insert(Column("pid")); // Photo ID
    schema.push_back(filesTable);

    Table tagsTable;
    tagsTable.name = "tags";
    tagsTable.columns.insert(Column("pid"));
    tagsTable.columns.insert(Column("tag"));
    schema.push_back(tagsTable);

    m_db = new Database();
    m_db->open();

    bool created;
    created = m_db->checkSchema(schema);

    if (created)
    {
        m_db->execute("CREATE UNIQUE INDEX IF NOT EXISTS tags_uni_idx ON tags (pid, tag)");
    }
}

Index::~Index()
{
    delete m_db;
}

bool Index::scanDirectory(string dir)
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
            scanDirectory(path);
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

            vector<string> args;
            args.push_back(path);
            ResultSet rs;
            rs = m_db->executeQuery("SELECT * FROM files WHERE path=?", args);
            if (rs.rows.size() == 0)
            {
                File f(path);
                f.scan();

                // File hasn't been seen before
                string fp;
                fp = f.getFingerprint();
                printf("%s: %-32s\n", fp.c_str(), path.c_str());

                // See if we've seen this fingerprint before
                args.clear();
                args.push_back(fp);
                rs = m_db->executeQuery("SELECT * FROM photos WHERE pid=?", args);

                m_db->startTransaction();
                if (rs.rows.size() == 0)
                {
                    // Extract details from the file
                    set<string> tags;
                    time_t timestamp;
                    f.getTags(tags, &timestamp);

                    tags.insert("Fotofing/Visible");

                    printf("Index::scanDirectory: timestamp=%ld\n", timestamp);
                    Surface* thumbnail = f.getThumbnail();

                    uint8_t* thumbData = NULL;
                    unsigned long thumbLength = 0;
                    thumbnail->saveJPEG(&thumbData, &thumbLength);

                    int res;
                    sqlite3_stmt* stmt;
                    string insertPhoto = "INSERT INTO photos (pid, thumbnail, timestamp) VALUES (?, ?, ?)";
                    res = sqlite3_prepare_v2(m_db->getDB(), insertPhoto.c_str(), insertPhoto.length(), &stmt, NULL);
                    if (res)
                    {
                        printf("Database::open: Error: %s\n", sqlite3_errmsg(m_db->getDB()));
                        printf("Index::scanDirectory: Failed to prepare statement: %d\n", res);
                        return false;
                    }
                    sqlite3_bind_text(stmt, 1, fp.c_str(), fp.length(), SQLITE_TRANSIENT);
                    sqlite3_bind_blob(stmt, 2, thumbData, thumbLength, SQLITE_TRANSIENT);
                    sqlite3_bind_int64(stmt, 3, timestamp);

                    res = sqlite3_step(stmt);
                    //printf("Index::scanDirectory: res=%d\n", res);
                    sqlite3_finalize(stmt);
                    if (res != SQLITE_DONE)
                    {
                        printf("Index::scanDirectory: Unexpected result: res=%d\n", res);
                        return false;
                    }

                    free(thumbData);

                    saveTags(fp, tags);
                }

                args.clear();
                args.push_back(path);
                args.push_back(fp);
                m_db->execute("INSERT INTO files (path, pid) VALUES (?, ?)", args);
                m_db->endTransaction();
            }
        }
    }
    closedir(fd);
    return true;
}

bool Index::saveTags(string pid, set<string> tags)
{
    set<string> parentTags;
    set<string>::iterator it;

    // Extract all the parent tags
    for (it = tags.begin(); it != tags.end(); it++)
    {
        string tag = *it;
        size_t pos = tag.npos;
        while (true)
        {
            pos = tag.rfind('/', pos);
            if (pos != tag.npos)
            {
                string parent = tag.substr(0, pos);
                if (parent.length() > 0)
                {
                    parentTags.insert(parent);
                }
                pos--;
                if (pos <= 0)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }

    for (it = parentTags.begin(); it != parentTags.end(); it++)
    {
        tags.insert(*it);
    }

    PreparedStatement* ps = m_db->prepareStatement("INSERT INTO tags (pid, tag) VALUES (?, ?)");

    m_db->startTransaction();
    for (it = tags.begin(); it != tags.end(); it++)
    {
        string tag = *it;
        ps->bindString(1, pid);
        ps->bindString(2, tag);
        ps->execute();
    }
    m_db->endTransaction();

    delete ps;

    return true;
}

set<string> Index::getAllTags()
{
    set<string> tags;

    ResultSet rs = m_db->executeQuery("SELECT DISTINCT tag FROM tags");
    vector<Row>::iterator it;

    for (it = rs.rows.begin(); it != rs.rows.end(); it++)
    {
        tags.insert(it->getValue("tag"));
    }

    return tags;
}

set<string> Index::getTags(string pid)
{
    set<string> tags;

    PreparedStatement* ps = m_db->prepareStatement(
        "SELECT tag FROM tags WHERE pid=?");

    ps->bindString(1, pid);
    ps->executeQuery();

    while (ps->step())
    {
        tags.insert(ps->getString(0));
    }
    delete ps;

    // Only return leaf tags (ie those without children)
    set<string>::iterator it1;
    bool foundChild = true;
    while (foundChild)
    {
        for (it1 = tags.begin(); it1 != tags.end(); it1++)
        {
            string tag1 = *it1;
            string tag1slash = tag1 + "/";

            foundChild = false;
            set<string>::iterator it2;
            for (it2 = tags.begin(); it2 != tags.end(); it2++)
            {
                string tag2 = *it2;
                if (tag2.length() > tag1.length())
                {
                    string tag2start = tag2.substr(0, tag1slash.length());
                    if (tag2start == tag1slash)
                    {
                        foundChild = true;
                        break;
                    }
                }
            }
            if (foundChild)
            {
                tags.erase(it1);
                break;
            }
        }
    }

    return tags;
}

static Photo* createPhoto(PreparedStatement* ps)
{
     string pid;
     const void* thumbnailData;
     uint32_t thumbnailBytes;
     int64_t timestamp;

     pid = ps->getString(0);
     ps->getBlob(1, &thumbnailData, &thumbnailBytes);
     timestamp = ps->getInt64(2);

     Surface* thumbnail = Surface::loadJPEG(
         (uint8_t*)thumbnailData,
         thumbnailBytes);

     Photo* p = new Photo(pid, thumbnail, timestamp);
     return p;
}

vector<Photo*> Index::getPhotos()
{
    vector<Photo*> results;
    PreparedStatement* ps = m_db->prepareStatement(
        "SELECT pid, thumbnail, timestamp FROM photos");

    ps->executeQuery();

    while (ps->step())
    {
        Photo* p = createPhoto(ps);
        results.push_back(p);
    }
    delete ps;

    return results;
}

vector<Photo*> Index::getPhotos(vector<string> tags)
{
    if (tags.size() == 0)
    {
        return getPhotos();
    }

    vector<Photo*> results;
    string sql = "";
    sql += "SELECT pid, thumbnail, timestamp FROM photos";
    sql += " WHERE";
    bool addAnd = false;
    vector<string>::iterator it;
    for (it = tags.begin(); it != tags.end(); it++)
    {
        if (addAnd)
        {
            sql += " AND";
        }
        addAnd = true;
        sql += " pid IN (SELECT pid FROM tags WHERE tag = ?)";
    }

#if 0
    printf("Index::getPhotos: sql=%s\n", sql.c_str());
#endif
    PreparedStatement* ps = m_db->prepareStatement(sql);

    int i = 1;
    for (it = tags.begin(); it != tags.end(); it++)
    {
        ps->bindString(i, *it);
        i++;
    }

    ps->executeQuery();

    while (ps->step())
    {
        Photo* p = createPhoto(ps);
        results.push_back(p);
    }
    delete ps;

    return results;
}



