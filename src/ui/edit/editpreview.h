#ifndef __FOTOFING_UI_EDIT_PREVIEW_H_
#define __FOTOFING_UI_EDIT_PREVIEW_H_

#include <gtkmm.h>

#include <fotofing/workflow.h>

class EditPreview : public Gtk::DrawingArea
{
 private:
    Workflow* m_workflow;
    Geek::Gfx::Surface* m_original;
    Geek::Gfx::Surface* m_rendered;

 public:
    EditPreview();
    ~EditPreview();

    void setWorkflow(Workflow* workflow);

    void render();

    virtual void on_size_allocate(Gtk::Allocation& allocation);
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
};

#endif
