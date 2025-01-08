#include <logger.h>
#include <stdio.h>
#include <stdarg.h>

void mato_logger(log_level_t level, const char* file, int line,
                    const char* fmt, ...) 
{
    const char* level_str = NULL;
    switch(level) {
        case MATO_LOG_DEBUG: level_str = "DEBUG";  break;
        case MATO_LOG_INFO:  level_str = "INFO ";  break;
        case MATO_LOG_WARN:  level_str = "WARN ";  break;
        case MATO_LOG_ERROR: level_str = "ERROR";  break;
        default:        level_str = "?????";  break;
    }

    fprintf(stderr, "[Logger][%s] %s:%d: ", level_str, file, line);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");
}