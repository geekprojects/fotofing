#ifndef __FOTOFING_UI_UI_H_
#define __FOTOFING_UI_UI_H_

#include <gtkmm.h>

#include <dialogs/about.h>

class FotofingUI : public Gtk::Application
{
 private:

    About m_about;

    bool init();

 public:
    FotofingUI(int argc, char** argv);
    virtual ~FotofingUI();

};

#endif
