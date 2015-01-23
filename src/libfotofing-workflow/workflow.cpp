
#include <fotofing/workflow.h>

void Workflow::addOperation(OperationInstance* op)
{
    m_operations.push_back(op);
}

