#ifndef __FOTOFING_UI_EDIT_OPERATIONWIDGET_H_
#define __FOTOFING_UI_EDIT_OPERATIONWIDGET_H_

#include <gtkmm.h>
#include <map>

#include <fotofing/workflow.h>

class Edit;

class OperationWidget : public Gtk::Expander
{
 private:
    Edit* m_edit;
    OperationInstance* m_op;

    Gtk::VBox m_attrsBox;
    std::map<OperationAttribute, Gtk::Widget*> m_attrs;

    void createAttributes();

 public:
    OperationWidget(Edit* edit, OperationInstance* op);
    virtual ~OperationWidget();

    OperationInstance* getOperationInstance();

    bool onAttrButtonRelease(GdkEventButton* button);

    void updateValues();
};

#endif
