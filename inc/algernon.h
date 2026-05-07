#ifndef ALGERNON_H_
#define ALGERNON_H_

/**
 * @file algernon.h
 * @brief Umbrella header: include this single file to import all Algernon modules.
 */

#include "version.h"
#include "log/xerror.h"
#include "log/xlogger.h"
#include "math/xmath.h"
#include "memory/xbuffer.h"
#include "file/xpath.h"
#include "file/xfile.h"
#include "regex/xregex.h"
#include "json/xjson.h"
#include "perf/xtimer.h"
#include "perf/xtracer.h"
#include "sys/xdlib.h"
#include "sys/xplatform.h"
#include "cv/ximage.h"
#include "cv/ximage_io.h"
#include "util/xargs.h"
#include "threadpool/xthreadpool.h"
#include "threadpool/xthread_flow.h"

#endif // ALGERNON_H_
