#include "MYTEST.h"
#include "myUtils/logger.h"

int main()
{
    Logger::instance()->open("./main.log");

    debug("name=%s age=%d", "jack", 18);

    test_OpenCL();
}
