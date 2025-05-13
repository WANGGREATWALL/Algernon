/** Usage:
 * @brief User should define TAG_LOGGER in project, for example:
 * 
 * #define TAG_LOGGER "[TestAlgo]"
 * #include "logger.h"
 * volatile int G_LEVEL_LOGGER = LEVEL_LOGGER_DEFAULT
 * ...
 * 
 * */

#ifndef _LOGGER_H_
#define _LOGGER_H_


#include <stdio.h>
#ifdef __ANDROID__
#include <android/log.h>
#endif

#undef NDEBUG
// #define NDEBUG
#include <cassert>
#include <error.h>

#ifndef TAG_LOGGER
#define TAG_LOGGER "[unnamed]"
#endif

#define TAG_LOGGER_FATAL            "[fatal]"
#define TAG_LOGGER_ERROR            "[error]"
#define TAG_LOGGER_WARN             "[warn]"
#define TAG_LOGGER_INFO             "[info]"
#define TAG_LOGGER_debug            "[debug]"
#define TAG_LOGGER_VERBO            "[verbo]"

#define LEVEL_LOGGER_SILENT         (8)
#define LEVEL_LOGGER_FATAL          (7)
#define LEVEL_LOGGER_ERROR          (6)
#define LEVEL_LOGGER_WARN           (5)
#define LEVEL_LOGGER_INFO           (4)
#define LEVEL_LOGGER_DEBUG          (3)
#define LEVEL_LOGGER_VERBOSE        (2)
#define LEVEL_LOGGER_DEFAULT        (1)
#define LEVEL_LOGGER_UNKNOWN        (0)

extern volatile int G_LEVEL_LOGGER;

#ifdef __ANDROID__
#define LOGGER_IF(level, tag_level, fmt, ...) \
    if (level >= G_LEVEL_LOGGER) { \
        __android_log_print(level, TAG_LOGGER, fmt, ##__VA_ARGS__); \
        printf("%s %s " fmt, TAG_LOGGER, tag_level, ##__VA_ARGS__); \
    }
#else
#define LOGGER_IF(level, tag_level, fmt, ...) \
    if (level >= G_LEVEL_LOGGER) { \
        printf("%s %s " fmt, TAG_LOGGER, tag_level, ##__VA_ARGS__); \
    }
#endif

#define LOGGER_F(fmt, ...) LOGGER_IF(LEVEL_LOGGER_FATAL, TAG_LOGGER_FATAL, fmt, ##__VA_ARGS__)
#define LOGGER_E(fmt, ...) LOGGER_IF(LEVEL_LOGGER_ERROR, TAG_LOGGER_ERROR, fmt, ##__VA_ARGS__)
#define LOGGER_W(fmt, ...) LOGGER_IF(LEVEL_LOGGER_WARN, TAG_LOGGER_WARN, fmt, ##__VA_ARGS__)
#define LOGGER_I(fmt, ...) LOGGER_IF(LEVEL_LOGGER_INFO, TAG_LOGGER_INFO, fmt, ##__VA_ARGS__)
#define LOGGER_D(fmt, ...) LOGGER_IF(LEVEL_LOGGER_DEBUG, TAG_LOGGER_DEBUG, fmt, ##__VA_ARGS__)
#define LOGGER_V(fmt, ...) LOGGER_IF(LEVEL_LOGGER_VERBOSE, TAG_LOGGER_VERBO, fmt, ##__VA_ARGS__)

#define ASSERTER(expression) assert(expression)

#define ASSERTER_WITH_RET(expression, ret) \
	if (!(expression)) { \
		LOGGER_E("expression: '%s' failed! (%s:%d)\n", #expression, __FILE__, __LINE__); \
		return ret; \
	}

#define ASSERTER_WITH_INFO(expression, ret, fmt, ...) \
    if (!(expression)) { \
        LOGGER_E(fmt " (%s:%d)\n", ##__VA_ARGS__, __FILE__, __LINE__); \
        return ret; \
    }


#endif // _LOGGER_H_