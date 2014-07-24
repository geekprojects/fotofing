
#include "about.h"

using namespace std;

About::About()
{
    set_transient_for(*this);
    set_program_name("Fotofing");
    set_version("0.0.1");
    set_copyright("GeekProjects");
    set_comments("Photo Tagging and Management");
    set_license_type(Gtk::LICENSE_GPL_3_0);
    set_website("http://geekprojects.com");
    set_website_label("GeekProjects.com");

    std::vector<Glib::ustring> authors;
    authors.push_back("Ian Parker <ian@geekprojects.com>");
    set_authors(authors);

    signal_response().connect(sigc::mem_fun(*this, &About::close));
}

About::~About()
{
}

void About::open()
{
    show();
    present();
}

void About::close(int responseId)
{
    if ((responseId == Gtk::RESPONSE_CLOSE) ||
        (responseId == Gtk::RESPONSE_CANCEL) )
    {
        hide();
    }
}

