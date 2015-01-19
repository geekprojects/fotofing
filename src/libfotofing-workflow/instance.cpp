
#include <fotofing/workflow.h>

using namespace std;
using namespace Geek;
using namespace Geek::Gfx;

OperationInstance::OperationInstance()
{
}

OperationInstance::~OperationInstance()
{
}


void OperationInstance::setAttribute(std::string name, int i)
{
}

void OperationInstance::setAttribute(std::string name, double d)
{
}

void OperationInstance::apply(
    Geek::Gfx::Surface* surface,
    ProgressListener* prog)
{
}

// Common utility methods
void OperationInstance::convolution(Surface* surface, CentredMatrix* matrix)
{
}


