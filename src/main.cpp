#define TAG_LOGGER "[Algernon]"
#include "logger.h"
volatile int G_LEVEL_LOGGER = LEVEL_LOGGER_DEFAULT;

#include "xbuffer.h"
#include "xfile.h"
#include "xthread_flow.h"
#include "performance.h"


int main()
{
    perf::setTimerRootName("Algernon");
    perf::TracerScoped trace("main");

    file::XFile f("cmake_install.cmake");
    auto fbuffer = f.getBuffer();
    LOGGER_I("buffer.size = %lu\n", fbuffer.sizeByByte());

    int retInitFlow = framework::Flow::get().init(3, 2);
    ASSERTER_WITH_RET(retInitFlow == ECODE_SUCCESS, retInitFlow);

    auto pipeline = framework::Flow::get().addPipeline(
        file::exists, "cmake_install.cmake"
    );

    bool exist = pipeline.get();
    LOGGER_I("exist = %s\n", exist ? "true" : "false");

    return ECODE_SUCCESS;
}