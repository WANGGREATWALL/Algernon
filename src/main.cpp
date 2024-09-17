#define TAG_LOGGER "[Algernon]"
#include "logger.h"
volatile int G_LEVEL_LOGGER = LEVEL_LOGGER_DEFAULT;

#include "xbuffer.h"
#include "xfile.h"
#include "xthread_flow.h"
#include "performance.h"
#include "xjson.h"


int getJsonValue(const std::string& filename)
{
    json::XJson json(filename);

    LOGGER_I("json.name = %s\n", filename.c_str());
    LOGGER_I("json.isValid() = %s\n", json.isValid() ? "true" : "false");
    LOGGER_I("json[name] = %s\n", json["name"].getString().c_str());
    LOGGER_I("json[type] = %s\n", json["type"].getString().c_str());
    LOGGER_I("json[score] = %f\n", json["score"].getFloat());
    LOGGER_I("json[page] = %d\n", json["page"].getInt());
    LOGGER_I("json[readed] = %s\n", json["readed"].getBool() ? "true" : "false");

    LOGGER_I("json[keywords] is array: %s\n", json["keywords"].isArray() ? "true" : "false");
    LOGGER_I("json[keywords].size = %lu\n", json["keywords"].getArraySize());
    LOGGER_I("json[keywords] = \n");
    for (int i = 0; i < json["keywords"].getArraySize(); ++i) {
        LOGGER_I("json[keywords][%d] = %s\n", i, json["keywords"][i].getString().c_str());
    }
    
    return ECODE_SUCCESS;
}


int main()
{
    perf::setTimerRootName("Algernon");
    perf::TracerScoped trace("main");

    std::string jsonfile = "main.json";

    trace.sub("init framework");
    int retInitFlow = framework::Flow::get().init(3, 2);
    ASSERTER_WITH_RET(retInitFlow == ECODE_SUCCESS, retInitFlow);

    trace.sub("load file");
    bool exist = file::exists(jsonfile);
    LOGGER_I("exist = %s\n", exist ? "true" : "false");

    file::XFile f(jsonfile);
    auto fbuffer = f.getBuffer();
    LOGGER_I("buffer.size = %lu\n", fbuffer.sizeByByte());

    trace.sub("add pipeline");
    auto pipeline = framework::Flow::get().addPipeline(
        getJsonValue, jsonfile
    );

    int retGetJsonValue = pipeline.get();
    ASSERTER_WITH_RET(retGetJsonValue == ECODE_SUCCESS, retGetJsonValue);

    return ECODE_SUCCESS;
}