#define TAG_LOGGER "[Algernon]"
#include "log/logger.h"
volatile int G_LEVEL_LOGGER = LEVEL_LOGGER_DEFAULT;

#include "ext/trait_test.h"


int main()
{
    auto test = std::is_const<int>::value;


    return ECODE_SUCCESS;
}