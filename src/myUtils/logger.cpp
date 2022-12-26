#include "logger.h"
#include <time.h>
#include <stdarg.h>
#include <string.h>

namespace caddie {
    namespace logger {
        const char* Logger::levels[LEVEL_COUNT] =
        {
            "DEBUG",
            "INFO",
            "WARN",
            "ERROR",
            "FATAL"
        };

        Logger::Logger() : _level(DEBUG) {}
        Logger::~Logger() = default;

        Logger& Logger::get()
        {
            static Logger instance;
            return instance;
        }

        void Logger::log(enum Level level, const char* file, int line, const char* format, ...)
        {
            if (_level > level)
            {
                return;
            }

            time_t ticks = time(NULL);
            struct tm time;
            localtime_s(&time, &ticks);
            char timestamp[32];
            memset(timestamp, 0, sizeof(timestamp));
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &time);

            int len = 0;
            const char* fmt = "%s %s %s:%d ";
            len = snprintf(NULL, 0, fmt, timestamp, levels[level], file, line);
            if (len > 0)
            {
                char* buffer = new char[len + 1];
                snprintf(buffer, len + 1, fmt, timestamp, levels[level], file, line);
                buffer[len] = 0;
                delete[] buffer;
            }

            va_list arg_ptr;
            va_start(arg_ptr, format);
            len = vsnprintf(NULL, 0, format, arg_ptr);
            va_end(arg_ptr);
            if (len > 0)
            {
                char* content = new char[len + 1];
                va_start(arg_ptr, format);
                vsnprintf(content, len + 1, format, arg_ptr);
                va_end(arg_ptr);
                content[len] = 0;
                delete[] content;
            }
        }

        void Logger::setlevel(int level)
        {
            _level = level;
        }
    }
}
