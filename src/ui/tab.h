#ifndef __FOTOFING_UI_TAB_H_
#define __FOTOFING_UI_TAB_H_

class Tab : public Gtk::Box
{
 protected:

 public:
    Tab(Gtk::Orientation orientation) : Gtk::Box(orientation)
    {
    }

    virtual ~Tab()
    {
    }

    virtual Gtk::Widget* getTabLabel() = 0;
    virtual Glib::RefPtr< Gio::Menu >& getMenu() = 0;
};

#endif
