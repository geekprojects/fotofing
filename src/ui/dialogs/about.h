#ifndef __FOTOFING_UI_ABOUT_H_
#define __FOTOFING_UI_ABOUT_H_

#include <gtkmm.h>

class About : public Gtk::AboutDialog
{
 private:

 public:
    About();
    ~About();

    void open();
    void close(int responseId);
};

#endif
