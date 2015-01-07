
#include "edit.h"

Edit::Edit() :
    m_label("Edit tab")
{
    pack_start(m_label, Gtk::PACK_EXPAND_WIDGET);
}

Edit::~Edit()
{
}



