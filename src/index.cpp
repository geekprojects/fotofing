
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
    photosTable.columns.insert(Column("pid", true)); // Photo ID
    photosTable.columns.insert(Column("thumbnail", true)); // Image Blob
    schema.push_back(photosTable);

    Table filesTable;
    filesTable.name = "files";
    filesTable.columns.insert(Column("path", true));
    filesTable.columns.insert(Column("pid")); // Photo ID
    schema.push_back(filesTable);

    m_db = new Database();
    m_db->open();
    m_db->checkSchema(schema);
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
            printf("Matching files: %lu\n", rs.rows.size());
            if (rs.rows.size() == 0)
            {
                // File hasn't been seen before
                string fp;
                fp = fingerprint(path);
                printf("%s: %-32s\n", fp.c_str(), path.c_str());

                // See if we've seen this fingerprint before
                args.clear();
                args.push_back(fp);
                rs = m_db->executeQuery("SELECT * FROM photos WHERE pid=?", args);
                printf("Matching PIDs: %lu\n", rs.rows.size());

                if (rs.rows.size() == 0)
                {
                    // New fingerprint!
                    printf("New fingerprint!\n");

                    File f(path);
                    Surface* thumbnail = f.generateThumbnail();

                    uint8_t* thumbData = NULL;
                    unsigned long thumbLength = 0;
                    thumbnail->saveJPEG(&thumbData, &thumbLength);
printf("Index::scanDirectory: thumbData=%p, thumbLength=%lu\n", thumbData, thumbLength);

                    int res;
                    sqlite3_stmt* stmt;
                    string insertPhoto = "INSERT INTO photos (pid, thumbnail) VALUES (?, ?)";
                    res = sqlite3_prepare_v2(m_db->getDB(), insertPhoto.c_str(), insertPhoto.length(), &stmt, NULL);
                    if (res)
                    {
                        printf("Index::scanDirectory: Failed to prepare statement: %d\n", res);
                        return false;
                    }
                    sqlite3_bind_text(stmt, 1, fp.c_str(), fp.length(), SQLITE_TRANSIENT);
                    sqlite3_bind_blob(stmt, 2, thumbData, thumbLength, SQLITE_TRANSIENT);

                    res = sqlite3_step(stmt);
                    printf("Index::scanDirectory: res=%d\n", res);
                    sqlite3_finalize(stmt);
                    if (res != SQLITE_DONE)
                    {
                        printf("Index::scanDirectory: Unexpected result: res=%d\n", res);
                        return false;
                    }

                    free(thumbData);
                }

                args.clear();
                args.push_back(path);
                args.push_back(fp);
                m_db->execute("INSERT INTO files (path, pid) VALUES (?, ?)", args);
            }
        }

    }
    closedir(fd);
    return true;
}

vector<Photo> Index::getPhotos()
{
vector<Photo> results;
    sqlite3_stmt* stmt;
    string SELECT_PHOTOS_SQL = "SELECT pid, thumbnail FROM photos";
    sqlite3_prepare_v2(m_db->getDB(), SELECT_PHOTOS_SQL.c_str(), SELECT_PHOTOS_SQL.length(), &stmt, NULL);

    while (true)
{
int s;
s = sqlite3_step(stmt);
if (s == SQLITE_ROW)
{
const unsigned char* pid;
pid = sqlite3_column_text(stmt, 0);
const void* thumbnailData;
uint32_t thumbnailBytes;
thumbnailData = sqlite3_column_blob(stmt, 1);
thumbnailBytes = sqlite3_column_bytes(stmt, 1);

Surface* thumbnail = Surface::loadJPEG((uint8_t*)thumbnailData, thumbnailBytes);

        Photo p(string((char*)pid), thumbnail);
        results.push_back(p);
}
else if (s == SQLITE_DONE)
{
break;
}
else
{
printf("Index::getPhotos: Failed to retrieve photos: %d\n", s);
break;
}
    }
    sqlite3_finalize(stmt);

    return results;
}

