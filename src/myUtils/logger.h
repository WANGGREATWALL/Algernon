#pragma once
#include <string>

#define LOG_DEBUG(format, ...)  caddie::logger::Logger::get().log(caddie::logger::DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)   caddie::logger::Logger::get().log(caddie::logger::INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)   caddie::logger::Logger::get().log(caddie::logger::WARN, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...)  caddie::logger::Logger::get().log(caddie::logger::ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...)  caddie::logger::Logger::get().log(caddie::logger::FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_LEVEL(level)        caddie::logger::Logger::get().setlevel(level);

namespace caddie {

    namespace logger {

        enum Level
        {
            DEBUG       = 0,
            INFO        = 1,
            WARN        = 2,
            ERROR       = 3,
            FATAL       = 4,
            LEVEL_COUNT = 5
        };

        class Logger
        {
        public:
            static Logger& get();
            void log(Level level, const char* file, int line, const char* format, ...);
            void setlevel(int level);

        private:
            Logger();
            ~Logger();

        private:
            int _level;
            static const char* levels[LEVEL_COUNT];
        };
    } //namespace logger

}