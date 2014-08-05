
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fotofing/index.h>
#include <fotofing/file.h>

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
    filesTable.columns.insert(Column("source_id")); // Source ID
    schema.push_back(filesTable);

    Table tagsTable;
    tagsTable.name = "tags";
    tagsTable.columns.insert(Column("pid"));
    tagsTable.columns.insert(Column("tag"));
    schema.push_back(tagsTable);

    Table sourcesTable;
    sourcesTable.name = "sources";
    sourcesTable.columns.insert(Column("source_id", "INTEGER", true));
    sourcesTable.columns.insert(Column("type"));
    sourcesTable.columns.insert(Column("host"));
    sourcesTable.columns.insert(Column("path"));
    schema.push_back(sourcesTable);

    m_db = new Database();
    m_db->open();

    bool created;
    created = m_db->checkSchema(schema);

    if (created)
    {
        m_db->execute("CREATE UNIQUE INDEX IF NOT EXISTS tags_uni_idx ON tags (pid, tag)");
        m_db->execute("CREATE UNIQUE INDEX IF NOT EXISTS sources_uni_idx ON sources (host, path)");
    }
}

Index::~Index()
{
    delete m_db;
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

    PreparedStatement* ps = m_db->prepareStatement(
        "INSERT INTO tags (pid, tag) VALUES (?, ?)");

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

bool Index::removeTag(string pid, string tag)
{
    m_db->startTransaction();

    PreparedStatement* ps = m_db->prepareStatement(
        "DELETE FROM tags WHERE pid=? AND tag LIKE ?");
    ps->bindString(1, pid);
    ps->bindString(2, tag);
    ps->execute();

    ps->bindString(1, pid);
    ps->bindString(2, tag + "/%");
    ps->execute();
    delete ps;

    m_db->endTransaction();

    return true;
}

bool Index::removeTag(string tag)
{
    PreparedStatement* ps = m_db->prepareStatement("DELETE FROM tags WHERE tag LIKE ?");
    ps->bindString(1, tag);
    ps->execute();

    ps->bindString(1, tag + "/%");
    ps->execute();
    delete ps;

    return true;
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

vector<Photo*> Index::getPhotos(time_t* fromDate, time_t* toDate)
{
    vector<string> noTags;
    return getPhotos(noTags, fromDate, toDate);
}

vector<Photo*> Index::getPhotos(vector<string> tags, time_t* fromDate, time_t* toDate)
{
    vector<Photo*> results;
    string sql = "";
    sql += "SELECT pid, thumbnail, timestamp FROM photos";

    bool addAnd = false;

    if (fromDate != NULL && (*fromDate) != 0)
    {
        if (!addAnd)
        {
            sql += " WHERE";
        }
        addAnd = true;
        sql += " timestamp >= ?";
    }
    if (toDate != NULL && (*toDate) != 0)
    {
        if (!addAnd)
        {
            sql += " WHERE";
        }
        else
        {
            sql += " AND";
        }
        addAnd = true;
        sql += " timestamp <= ?";
    }

    vector<string>::iterator it;
    for (it = tags.begin(); it != tags.end(); it++)
    {
        if (!addAnd)
        {
            sql += " WHERE";
        }
        else
        {
            sql += " AND";
        }
        addAnd = true;
        sql += " pid IN (SELECT pid FROM tags WHERE tag = ?)";
    }
    sql += " ORDER BY timestamp ASC";

#if 1
    printf("Index::getPhotos: sql=%s\n", sql.c_str());
#endif

    PreparedStatement* ps = m_db->prepareStatement(sql);
    if (ps == NULL)
    {
        return results;
    }

    int i = 1;
    if (fromDate != NULL && (*fromDate) != 0)
    {
        ps->bindInt64(i++, *fromDate);
    }
    if (toDate != NULL && (*toDate) != 0)
    {
        // TODO: This needs to round up to the end of the day!
        ps->bindInt64(i++, *toDate);
    }
    for (it = tags.begin(); it != tags.end(); it++)
    {
        ps->bindString(i, *it);
        i++;
    }

    ps->executeQuery();

    *fromDate = time(NULL);
    *toDate = 0;

    while (ps->step())
    {
        Photo* p = createPhoto(ps);
        time_t t = p->getTimestamp();
        if (t < *fromDate)
        {
            *fromDate = t;
        }
        if (t > *toDate)
        {
            *toDate = t;
        }
        results.push_back(p);
    }
    delete ps;

    return results;
}

vector<File*> Index::getFiles(string pid)
{
    vector<File*> files;

    PreparedStatement* ps;
    ps = m_db->prepareStatement("SELECT path FROM files WHERE pid=?");
    ps->bindString(1, pid);

    ps->executeQuery();

    while (ps->step())
    {
        string path = ps->getString(0);
        File* f = new File(path);
        files.push_back(f);
    }

    return files;
}

bool Index::addFileSource(string path)
{
    FileSource fileSource(0, getHostName(), path);
    saveSource(&fileSource);

    return scanSource(&fileSource);
}

bool Index::scanFile(Source* source, File* f)
{
    printf("Index::scanFile: file=%s\n", f->getPath().c_str());
    vector<string> args;
    args.push_back(f->getPath());
    ResultSet rs;
    rs = m_db->executeQuery("SELECT * FROM files WHERE path=?", args);
    if (rs.rows.size() == 0)
    {
        f->scan();

        // File hasn't been seen before
        string fp;
        fp = f->getFingerprint();
        printf("%s: %-32s\n", fp.c_str(), f->getPath().c_str());

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
            f->getTags(tags, &timestamp);

            tags.insert("Fotofing/Visible");

            printf("Index::scanDirectory: timestamp=%ld\n", timestamp);
            Surface* thumbnail = f->getThumbnail();

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

        PreparedStatement* ps;
        ps = m_db->prepareStatement(
            "INSERT INTO files (path, pid, source_id) VALUES (?, ?, ?)");
        ps->bindString(1, f->getPath());
        ps->bindString(2, fp);
        ps->bindInt64(3, source->getSourceId());
        ps->execute();
        delete ps;
        m_db->endTransaction();
    }

    return true;
}

bool Index::scanSource(Source* s)
{
    return s->scan(this);
}

bool Index::saveSource(Source* s)
{
    PreparedStatement* ps;
    int i = 1;
    if (s->getSourceId() > 0)
    {
        ps = m_db->prepareStatement("INSERT INTO sources (source_id, type, host, path) VALUES (?, ?, ?, ?)");
        ps->bindInt64(i++, s->getSourceId());
    }
    else
    {
        ps = m_db->prepareStatement("INSERT INTO sources (type, host, path) VALUES (?, ?, ?)");
    }
    ps->bindString(i++, s->getType());
    ps->bindString(i++, s->getHost());
    ps->bindString(i++, s->getPath());

    bool res;
    res = ps->execute();

    if (res && s->getSourceId() == 0)
    {
        int64_t rowId = m_db->getLastInsertId();
        printf("Index::saveSource: Last insert id=%ld\n", rowId);
        s->setSourceId(rowId);
    }
    delete ps;

    return res;
}

vector<Source> Index::getSources()
{
    vector<Source> sources;
    PreparedStatement* ps;
    ps = m_db->prepareStatement("SELECT source_id, type, host, path FROM sources");

    ps->executeQuery();
    while (ps->step())
    {
        int64_t source_id = ps->getInt64(0);
        string type = ps->getString(1);
        string host = ps->getString(2);
        string path = ps->getString(3);
        if (type == "File")
        {
            FileSource f(source_id, host, path);
            sources.push_back(f);
        }
        else
        {
            Source f(source_id, type, host, path);
            sources.push_back(f);
        }
    }
    delete ps;

    return sources;
}


