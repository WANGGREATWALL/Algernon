#ifndef ALGERNON_H_
#define ALGERNON_H_

/**
 * @file algernon.h
 * @brief Umbrella header: include this single file to import all Algernon modules.
 */

#include "algernon/version.h"
#include "algernon/log/xerror.h"
#include "algernon/log/xlogger.h"
#include "algernon/math/xmath.h"
#include "algernon/memory/xbuffer.h"
#include "algernon/file/xpath.h"
#include "algernon/file/xfile.h"
#include "algernon/regex/xregex.h"
#include "algernon/json/xjson.h"
#include "algernon/perf/xtimer.h"
#include "algernon/perf/xtracer.h"
#include "algernon/sys/xdlib.h"
#include "algernon/sys/xplatform.h"
#include "algernon/cv/ximage.h"
#include "algernon/cv/ximage_io.h"
#include "algernon/util/xargs.h"
#include "algernon/threadpool/xthreadpool.h"
#include "algernon/threadpool/xthread_flow.h"

#endif // ALGERNON_H_
