
#include <fotofing/workflow.h>

using namespace std;

void Workflow::addOperation(OperationInstance* op)
{
    m_operations.push_back(op);
}

void Workflow::deleteOperation(OperationInstance* opr)
{
    vector<OperationInstance*>::iterator it;
    for (it = m_operations.begin(); it != m_operations.end(); it++)
    {
        OperationInstance* op = *it;
        if (opr == op)
        {
            m_operations.erase(it);
            break;
        }
    }
}

