
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stat.h>

#include <fotofing/index.h>
#include <fotofing/file.h>

using namespace std;
using namespace Geek::Gfx;

Index::Index(string path)
{
    m_path = path;

}

Index::~Index()
{
    delete m_db;
}

bool Index::open()
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
    tagsTable.columns.insert(Column("type"));
    tagsTable.columns.insert(Column("data"));
    schema.push_back(tagsTable);

    Table sourcesTable;
    sourcesTable.name = "sources";
    sourcesTable.columns.insert(Column("source_id", "INTEGER", true));
    sourcesTable.columns.insert(Column("type"));
    sourcesTable.columns.insert(Column("host"));
    sourcesTable.columns.insert(Column("path"));
    schema.push_back(sourcesTable);

    m_db = new Database(m_path);
    bool res = m_db->open();
    if (!res)
    {
        return false;
    }

    bool created;
    created = m_db->checkSchema(schema);

    if (created)
    {
        m_db->execute("CREATE UNIQUE INDEX IF NOT EXISTS tags_uni_idx ON tags (pid, tag)");
        m_db->execute("CREATE UNIQUE INDEX IF NOT EXISTS sources_uni_idx ON sources (host, path)");
    }
    return true;
}

bool Index::saveTags(string pid, map<string, TagData*> tags)
{
    map<string, TagData*> parentTags;
    map<string, TagData*>::iterator it;

    // Extract all the parent tags
    for (it = tags.begin(); it != tags.end(); it++)
    {
        string tag = it->first;
        size_t pos = tag.npos;
        while (true)
        {
            pos = tag.rfind('/', pos);
            if (pos != tag.npos)
            {
                string parent = tag.substr(0, pos);
                if (parent.length() > 0)
                {
                    parentTags.insert(make_pair(parent, (TagData*)NULL));
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
        "INSERT OR REPLACE INTO tags (pid, tag, type, data) VALUES (?, ?, ?, ?)");

    m_db->startTransaction();
    for (it = tags.begin(); it != tags.end(); it++)
    {
        string tag = it->first;
        TagData* tagData = it->second;
        ps->bindString(1, pid);
        ps->bindString(2, tag);
        if (tagData != NULL)
        {
            ps->bindInt64(3, tagData->type);
            switch (tagData->type)
            {
                case SQLITE_INTEGER:
                    ps->bindInt64(4, tagData->data.i);
                    break;

                case SQLITE_TEXT:
                    ps->bindString(4, (const char*)tagData->data.blob.data, tagData->data.blob.length);

                case SQLITE_BLOB:
                    ps->bindBlob(4, tagData->data.blob.data, tagData->data.blob.length);
                    break;

                default:
                    ps->bindNull(4);
                    break;
            }
            delete tagData;
        }
        else
        {
            ps->bindInt64(3, SQLITE_NULL);
            ps->bindNull(4);
        }
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

set<string> Index::getChildTags(string tag)
{
    set<string> tags;

    PreparedStatement* ps = m_db->prepareStatement(
        "SELECT DISTINCT tag FROM tags WHERE tag LIKE ?");
    ps->bindString(1, tag + "/%");

    ps->executeQuery();
    while (ps->step())
    {
        tags.insert(ps->getString(0));
    }
    delete ps;

    return tags;
}

TagData* Index::getTagData(string pid, string tag)
{
    TagData* data = new TagData();

    PreparedStatement* ps = m_db->prepareStatement("SELECT type, data FROM tags WHERE pid = ? AND tag = ?");
    ps->bindString(1, pid);
    ps->bindString(2, tag);

    ps->executeQuery();
    if (ps->step())
    {
        int type = ps->getInt(0);
        switch (type)
        {
            case SQLITE_INTEGER:
                data->type = SQLITE_INTEGER;
                data->data.i = ps->getInt(1);
                break;

            case SQLITE_TEXT:
            {
                data->type = SQLITE_TEXT;
                string text = ps->getString(1);
                data->data.blob.data = strdup(text.c_str());
                data->data.blob.length = text.length();
            } break;

            case SQLITE_BLOB:
            {
                const void* blobData;
                uint32_t blobLength;
                ps->getBlob(1, &blobData, &blobLength);
                data->type = SQLITE_BLOB;
                data->data.blob.data = malloc(blobLength);
                memcpy(data->data.blob.data, blobData, blobLength);
                data->data.blob.length = blobLength;
            } break;

            default:
                printf("Index::getTagData: ERROR: Unhandled type: %d\n", type);
                break;
        }
    }
    delete ps;

    return data;
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

#if 0
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

    if (fromDate != NULL)
    {
        *fromDate = time(NULL);
    }
    if (toDate != NULL)
    {
        *toDate = 0;
    }

    while (ps->step())
    {
        Photo* p = createPhoto(ps);
        time_t t = p->getTimestamp();
        if (fromDate != NULL && t < *fromDate)
        {
            *fromDate = t;
        }
        if (toDate != NULL && t > *toDate)
        {
            *toDate = t;
        }
        results.push_back(p);
    }
    delete ps;

    return results;
}

Photo* Index::getPhoto(string pid)
{
    Photo* result = NULL;
    string sql = "SELECT pid, thumbnail, timestamp FROM photos WHERE pid=?";

    PreparedStatement* ps = m_db->prepareStatement(sql);
    if (ps == NULL)
    {
        return NULL;
    }

    ps->bindString(1, pid);

    ps->executeQuery();
    if (ps->step())
    {
        result = createPhoto(ps);
    }
    delete ps;

    return result;
}

bool Index::setProperty(string pid, string name, string value)
{
    map<string, TagData*> tags;
    tags.insert(make_pair("Fotofing/Properties/" + name, new TagData(value)));
    return saveTags(pid, tags);
}

string Index::getProperty(string pid, string name)
{
    TagData* data = getTagData(pid, "Fotofing/Properties/" + name);

    string result = "";

    if (data != NULL)
    {
        if ( data->type == SQLITE_TEXT)
        {
            result = string((const char*)data->data.blob.data, data->data.blob.length);
        }
        delete data;
    }

    return result;
}

vector<File*> Index::getFiles(string pid)
{
    vector<File*> files;

    PreparedStatement* ps;
    ps = m_db->prepareStatement("SELECT path, source_id FROM files WHERE pid=?");
    ps->bindString(1, pid);

    ps->executeQuery();

    while (ps->step())
    {
        string path = ps->getString(0);
        int64_t sourceId = ps->getInt64(1);
        File* f = new File(sourceId, path);
        files.push_back(f);
    }

    return files;
}

bool Index::addFileSource(string path)
{
    FileSource fileSource(0, getHostName(), path);
    return addSource(&fileSource);
}

bool Index::scanFile(Source* source, File* f)
{
#if 1
    printf("Index::scanFile: file=%s\n", f->getPath().c_str());
#endif
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
#if 0
        printf("%s: %-32s\n", fp.c_str(), f->getPath().c_str());
#endif

        // See if we've seen this fingerprint before
        args.clear();
        args.push_back(fp);
        rs = m_db->executeQuery("SELECT * FROM photos WHERE pid=?", args);

        m_db->startTransaction();
        if (rs.rows.size() == 0)
        {
            // Extract details from the file
            map<string, TagData*> tags;
            bool valid;

            valid = f->getTags(tags);
            if (!valid)
            {
                // Unable to extract tags!
                // Skip!
                m_db->endTransaction();
                return false;
            }

            // Look for the Date tag. It should contain the timestamp as data
            map<string, TagData*>::iterator it;
            time_t timestamp = 0;
            it = tags.find("Date");
            if (it != tags.end() && it->second != NULL)
            {
                timestamp = it->second->data.i;
            }
#if 0
            printf("Index::scanDirectory: timestamp=%ld\n", timestamp);
#endif

            // Mark the photo as visible
            tags.insert(make_pair("Fotofing/Visible", new TagData(1)));

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
                printf("Index::scanDirectory: Error: %s\n", sqlite3_errmsg(m_db->getDB()));
                printf("Index::scanDirectory: Failed to prepare statement: %d\n", res);
                return false;
            }
            sqlite3_bind_text(stmt, 1, fp.c_str(), fp.length(), SQLITE_TRANSIENT);
            sqlite3_bind_blob(stmt, 2, thumbData, thumbLength, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 3, timestamp);

            res = sqlite3_step(stmt);
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

bool Index::scanSource(Source* s, IndexClient* client)
{
    return s->scan(this, client);
}

bool Index::scanSources(IndexClient* client)
{
    vector<Source*> sources = getSources();
    vector<Source*>::iterator it;
    for (it = sources.begin(); it != sources.end(); it++)
    {
        (*it)->scan(this, client);
    }
    return true;
}

bool Index::addSource(Source* s)
{
    PreparedStatement* ps;
    int i = 1;
    if (s->getSourceId() > 0)
    {
        ps = m_db->prepareStatement("UPDATE sources SET host=?, path=? WHERE source_id=?");
        ps->bindString(i++, s->getHost());
        ps->bindString(i++, s->getPath());
        ps->bindInt64(i++, s->getSourceId());
    }
    else
    {
        ps = m_db->prepareStatement("INSERT INTO sources (type, host, path) VALUES (?, ?, ?)");
        ps->bindString(i++, s->getType());
        ps->bindString(i++, s->getHost());
        ps->bindString(i++, s->getPath());
    }

    bool res;
    res = ps->execute();

    if (res && s->getSourceId() == 0)
    {
        int64_t rowId = m_db->getLastInsertId();
#if 0
        printf("Index::addSource: Last insert id=%ld\n", rowId);
#endif
        s->setSourceId(rowId);
    }
    delete ps;

    return res;
}

bool Index::removeSource(Source* source)
{
    PreparedStatement* ps = m_db->prepareStatement(
        "DELETE FROM sources WHERE source_id=?");
    ps->bindInt64(1, source->getSourceId());

    bool res = ps->execute();
    if (!res)
    {
        printf(
            "Index::removeSource: Failed to remove source %ld\n",
            source->getSourceId());
    }

    return res;
}

vector<Source*> Index::getSources()
{
    vector<Source*> sources;
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
            FileSource* f = new FileSource(source_id, host, path);
            sources.push_back(f);
        }
        else
        {
            Source* f = new Source(source_id, type, host, path);
            sources.push_back(f);
        }
    }
    delete ps;

    return sources;
}

TagData::TagData()
{
    type = SQLITE_NULL;
    data.blob.data = NULL;
    data.blob.length = 0;
}

TagData::TagData(int64_t i)
{
    type = SQLITE_INTEGER;
    data.i = i;
}

TagData::TagData(const char* str)
{
    type = SQLITE_TEXT;
    data.blob.data = strdup(str);
    data.blob.length = strlen(str);
}

TagData::TagData(string str)
{
    type = SQLITE_TEXT;
    data.blob.data = strdup(str.c_str());
    data.blob.length = str.length();
}

TagData::TagData(void* tagdata, int length)
{
    type = SQLITE_BLOB;
    data.blob.data = malloc(length);
    memcpy(data.blob.data, tagdata, length);
    data.blob.length = length;
}

TagData::~TagData()
{
    if ((type == SQLITE_BLOB || type == SQLITE_TEXT) && data.blob.data != NULL)
    {
        free(data.blob.data);
    }
}

