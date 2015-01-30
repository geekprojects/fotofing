
#include <stdio.h>
#include <stdlib.h>

#include <fotofing/plugin.h>
#include <fotofing/workflow.h>

#include <dlfcn.h>

using namespace std;
using namespace Geek::Gfx;

int main(int argc, char** argv)
{

    vector<FotofingPlugin*> operations = FotofingPlugin::getPlugins("operation");
    vector<FotofingPlugin*>::iterator it;
    for (it = operations.begin(); it != operations.end(); it++)
    {
        Operation* op = (Operation*)*it;
        printf("Name: %s\n", op->getName().c_str());
        printf("Description: %s\n", op->getDescription().c_str());
        vector<OperationAttribute> attrs = op->getAttributes();
        vector<OperationAttribute>::iterator attrIt;
        for (
            attrIt = attrs.begin();
            attrIt != attrs.end();
            attrIt++)
        {
            OperationAttribute attr = *attrIt;
            printf("Attribute: %s\n", attr.name.c_str());
        }
    }

    return 0;
}


