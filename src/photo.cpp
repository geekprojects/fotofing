
#include "photo.h"

using namespace std;

Photo::Photo(string id, Surface* thumbnail)
{
    m_id = id;
    m_thumbnail = thumbnail;
}

Photo::~Photo()
{
    if (m_thumbnail != NULL)
    {
        delete m_thumbnail;
    }
}


