#include "MYTEST.h"
#include "myUtils/logger.h"

int main()
{
    LOG_LEVEL(caddie::logger::DEBUG);
    LOG_DEBUG("Hello, this is a test log!");
    LOG_INFO("name=%s age=%d", "jack", 18);

    //test_OpenCL();
    test_OpenGL();

    return EXIT_SUCCESS;
}
