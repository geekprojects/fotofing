#ifndef __FOTOFING_UI_EDIT_OPERATIONWIDGET_H_
#define __FOTOFING_UI_EDIT_OPERATIONWIDGET_H_

#include <gtkmm.h>
#include <map>

#include <fotofing/workflow.h>

class Edit;

class OperationWidget : public Gtk::VBox
{
 private:
    Edit* m_edit;
    OperationInstance* m_op;
    bool m_expanded;

    Gtk::Frame m_labelFrame;
    Gtk::Button m_expandButton;
    Gtk::HBox m_labelBox;
    Gtk::Label m_labelText;
    Gtk::Button m_labelDelete;

    Gtk::VBox m_attrsBox;
    std::map<OperationAttribute, Gtk::Widget*> m_attrs;

    void createAttributes();

 public:
    OperationWidget(Edit* edit, OperationInstance* op);
    virtual ~OperationWidget();

    OperationInstance* getOperationInstance();

    bool onAttrButtonRelease(GdkEventButton* button);
    void onDelete();
    void onExpand();

    void updateValues();
};

#endif
