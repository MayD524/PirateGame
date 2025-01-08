#pragma once

#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    MATO_LOG_DEBUG,
    MATO_LOG_INFO,
    MATO_LOG_WARN,
    MATO_LOG_ERROR
} log_level_t;

#ifdef LOGGING_DEBUG
  #warning DEFINED WITH LOGGING_DEBUG (DO NOT USE IN PROD)
  #define MATO_LOG(level, fmt, ...) \
      mato_logger(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
  /* In non-debug mode, you could disable logs or only keep errors, etc. */
  #define MATO_LOG(level, fmt, ...) \
      if ((level) == MATO_LOG_ERROR) { \
          mato_logger(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
      }
#endif

/**
 * The logging function for the thread pool, called by the THREADPOOL_LOG macro.
 * 
 * @param level   The log level (info, warn, error).
 * @param file    The source file name from which the log was invoked.
 * @param line    The line number in the source file.
 * @param fmt     The printf-style format string.
 * @param ...     The format arguments.
 */
void mato_logger(log_level_t level, const char* file, int line, 
                    const char* fmt, ...);

#endif