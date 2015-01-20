
#include <stdio.h>
#include <stdlib.h>

#include <fotofing/workflow.h>

#include <dlfcn.h>

using namespace std;
using namespace Geek::Gfx;

int main(int argc, char** argv)
{

void* handle = dlopen("../../plugins/operations/refocus/.libs/libfotofing-operation-refocus.so", RTLD_LAZY);

printf("workflow: handle=%p\n", handle);

if (handle == NULL)
{
printf("workflow: error: %s\n", dlerror());
return 0;
}

newOperation_t createOperation = (newOperation_t)dlsym(handle, "fotofing_operation_create");
printf("fotofing_operation_new: %p\n", createOperation);

Operation* op = createOperation();
printf("Name: %s\n", op->getName().c_str());
printf("Description: %s\n", op->getDescription().c_str());

OperationInstance* instance = op->createInstance();

Surface* srcSurface = Surface::loadJPEG("test.jpg");
instance->apply(srcSurface, NULL);
srcSurface->saveJPEG("output.jpg");
delete srcSurface;


dlclose(handle);

    return 0;
}


