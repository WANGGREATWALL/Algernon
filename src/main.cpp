#define TAG_LOGGER "[Algernon]"
#include "logger.h"
volatile int G_LEVEL_LOGGER = LEVEL_LOGGER_DEFAULT;

#include "xbuffer.h"
#include "xfile.h"


int main()
{
    bool exist = file::exists("cmake_install.cmake");
    LOGGER_I("exist = %s\n", exist ? "true" : "false");

    file::XFile f("cmake_install.cmake");
    auto fbuffer = f.getBuffer();
    LOGGER_I("buffer.size = %lu\n", fbuffer.sizeByByte());
    

    return ECODE_SUCCESS;
}