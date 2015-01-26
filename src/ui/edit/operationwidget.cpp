
#include "operationwidget.h"
#include "edit.h"

#include <vector>

using namespace std;

OperationWidget::OperationWidget(Edit* edit, OperationInstance* op)
{
    m_edit = edit;
    m_op = op;

    set_label(op->getOperation()->getName());
    add(m_attrsBox);

    createAttributes();

    show_all();
}

OperationWidget::~OperationWidget()
{
}

void OperationWidget::createAttributes()
{
    vector<OperationAttribute> attrs;
    attrs = m_op->getOperation()->getAttributes();

    vector<OperationAttribute>::iterator attrIt;
    for (attrIt = attrs.begin(); attrIt != attrs.end(); attrIt++)
    {
        OperationAttribute attr = *attrIt;
        Gtk::Label* attrLabel = Gtk::manage(new Gtk::Label(attr.label));
        m_attrsBox.pack_start(*attrLabel, Gtk::PACK_SHRINK);

        if (attr.type == OPERATION_ATTR_TYPE_INT)
        {
            int v = m_op->getAttributeInt(attr.name);
            Glib::RefPtr<Gtk::Adjustment> adjustment(
                Gtk::Adjustment::create(
                    v,
                    attr.min.i,
                    attr.max.i,
                    1,
                    10.0,
                    0.0) );
            Gtk::Scale* attrScale = Gtk::manage(new Gtk::HScale(adjustment));
            attrScale->set_digits(0);

            attrScale->set_value_pos(Gtk::POS_TOP);
            attrScale->set_draw_value();
            attrScale->signal_button_release_event().connect(
                sigc::mem_fun(*this, &OperationWidget::onAttrButtonRelease),
                false);
            m_attrsBox.pack_start(*attrScale, Gtk::PACK_SHRINK);
            m_attrs.insert(make_pair(attr, attrScale));
        }
        else if (attr.type == OPERATION_ATTR_TYPE_DOUBLE)
        {
            double v = m_op->getAttributeDouble(attr.name);
            Glib::RefPtr<Gtk::Adjustment> adjustment(
                Gtk::Adjustment::create(
                    attr.def.d,
                    attr.min.d,
                    attr.max.d,
                    0.01,
                    10.0,
                    0.0) );
            Gtk::Scale* attrScale = Gtk::manage(new Gtk::HScale(adjustment));
            attrScale->set_digits(2);

            attrScale->set_value_pos(Gtk::POS_TOP);
            attrScale->set_draw_value();
            attrScale->signal_button_release_event().connect(
                sigc::mem_fun(*this, &OperationWidget::onAttrButtonRelease),
                false);

            m_attrsBox.pack_start(*attrScale, Gtk::PACK_SHRINK);
            m_attrs.insert(make_pair(attr, attrScale));
        }
    }
}

OperationInstance* OperationWidget::getOperationInstance()
{
    return m_op;
}

bool OperationWidget::onAttrButtonRelease(GdkEventButton* event)
{
    updateValues();
    m_edit->onAttrChanged();
    return false;
}

void OperationWidget::updateValues()
{
    std::map<OperationAttribute, Gtk::Widget*>::iterator it;
    for (it = m_attrs.begin(); it != m_attrs.end(); it++)
    {
        OperationAttribute attr = it->first;
        Gtk::Widget* w = it->second;
        if (attr.type == OPERATION_ATTR_TYPE_INT)
        {
            Gtk::Scale* scale = (Gtk::Scale*)w;
            int v = (int)scale->get_value();
#if 0
            printf(
                "OperationWidget::getValues: %s: %s = (int)%d\n",
                m_op->getOperation()->getName().c_str(),
                attr.name.c_str(),
                v);
#endif
            m_op->setAttribute(attr.name, v);
        }
        else if (attr.type == OPERATION_ATTR_TYPE_DOUBLE)
        {
            Gtk::Scale* scale = (Gtk::Scale*)w;
            double v = scale->get_value();
#if 0
            printf(
                "OperationWidget::getValues: %s: %s = (double)%0.4g\n",
                m_op->getOperation()->getName().c_str(),
                attr.name.c_str(),
                v);
#endif
            m_op->setAttribute(attr.name, v);
        }
    }

}

