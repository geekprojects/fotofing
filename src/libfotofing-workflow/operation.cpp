
#include <fotofing/workflow.h>

using namespace std;

Operation::Operation()
{
}

Operation::~Operation()
{
}

string Operation::getName()
{
    return "no name";
}

string Operation::getDescription()
{
    return "";
}

vector<OperationAttribute> Operation::getAttributes()
{
    vector<OperationAttribute> attrs;
    return attrs;
}

OperationInstance* Operation::createInstance()
{
    return NULL;
}

const char* operationPaths[] = {
    "src/plugins/operations/refocus/.libs/",
    "/usr/lib",
    "/usr/local/lib",
    NULL
};

