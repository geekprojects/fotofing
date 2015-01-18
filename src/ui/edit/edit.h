#ifndef __FOTOFING_UI_EDIT_EDIT_H_
#define __FOTOFING_UI_EDIT_EDIT_H_

#include <gtkmm.h>

class Edit : public Gtk::VBox
{
 private:
    Gtk::Label m_label;

 public:
    Edit();
    ~Edit();
};

#endif
