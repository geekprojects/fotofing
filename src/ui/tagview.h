#ifndef __FOTOFING_UI_TAGVIEW_H_
#define __FOTOFING_UI_TAGVIEW_H_

struct Tag
{
    Tag()
    {
        parent = NULL;
    }

    ~Tag()
    {
        std::map<std::string, Tag*>::iterator it;
        for (it = children.begin(); it != children.end(); it++)
        {
            delete it->second;
        }
    }

    Tag* parent;
    std::string name;
    std::map<std::string, Tag*> children;
    Gtk::TreeRow treeRow;

    std::string getTagName()
    {
        std::string fullname = "";
        if (parent != NULL)
        {
            if (parent->parent != NULL)
            {
                fullname = parent->getTagName() + "/";
            }
        }
        fullname += name;
        return fullname;
    }
};

#endif
