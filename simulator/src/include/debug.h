// debug.h
#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#define DEBUG_ENTER(func) fprintf(stderr, ">>> Entering %s\n", func)
#define DEBUG_EXIT(func)  fprintf(stderr, "<<< Exiting  %s\n", func)

#define TRACE_FUNCTION() \
    DEBUG_ENTER(__func__); \
    do { } while (0)

#define TRACE_RETURN() \
    DEBUG_EXIT(__func__); \

#define DEBUG_LEVEL_NONE  0
#define DEBUG_LEVEL_ERROR 1
#define DEBUG_LEVEL_WARN  2
#define DEBUG_LEVEL_INFO  3
#define DEBUG_LEVEL_DEBUG 4

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_GRAY    "\033[1;90m"

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_LEVEL_INFO
#endif


#define LOG_ERROR(fmt, ...) \
    do { if (DEBUG_LEVEL >= DEBUG_LEVEL_ERROR) fprintf(stderr, COLOR_RED "[ERROR] %s:%d:%s(): " fmt COLOR_RESET "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); } while (0)

#define LOG_WARN(fmt, ...) \
    do { if (DEBUG_LEVEL >= DEBUG_LEVEL_WARN) fprintf(stderr, COLOR_YELLOW "[WARN]  %s:%d:%s(): " fmt COLOR_RESET "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); } while (0)

#define LOG_INFO(fmt, ...) \
    do { if (DEBUG_LEVEL >= DEBUG_LEVEL_INFO) fprintf(stderr, COLOR_GREEN "[INFO]  %s:%d:%s(): " fmt COLOR_RESET "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); } while (0)

#define LOG_DEBUG(fmt, ...) \
    do { if (DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG) fprintf(stderr, COLOR_CYAN "[DEBUG] %s:%d:%s(): " fmt COLOR_RESET "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); } while (0)

#endif 
