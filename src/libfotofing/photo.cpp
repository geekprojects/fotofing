
#include <fotofing/photo.h>

using namespace std;

Photo::Photo(string id, Surface* thumbnail, time_t timestamp)
{
    m_id = id;
    m_thumbnail = thumbnail;
    m_timestamp = timestamp;
}

Photo::~Photo()
{
    if (m_thumbnail != NULL)
    {
        delete m_thumbnail;
    }
}


