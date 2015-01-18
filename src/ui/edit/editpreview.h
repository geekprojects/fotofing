#ifndef __FOTOFING_UI_EDIT_PREVIEW_H_
#define __FOTOFING_UI_EDIT_PREVIEW_H_

#include <gtkmm.h>

#include <fotofing/workflow.h>

class EditPreview : public Gtk::DrawingArea
{
 private:
    Workflow* m_workflow;
    Geek::Gfx::Surface* m_surface;

 public:
    EditPreview();
    ~EditPreview();

    void setWorkflow(Workflow* workflow);

    void render();

    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
};

#endif
