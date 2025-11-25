/*
 * log.h
 *
 *  Created on: Jan 5, 2019
 *      Author: Thinpv
 */

#ifndef LOG_H__
#define LOG_H__

#include <stdint.h>
#include <stdarg.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	LOG_NONE, /*!< No log output */
	LOG_ERROR, /*!< Critical errors, software module can not recover on its own */
	LOG_WARN, /*!< Error conditions from which recovery measures have been taken */
	LOG_INFO, /*!< Information messages which describe normal flow of events */
	LOG_DEBUG, /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
	LOG_VERBOSE /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} log_level_t;

extern log_level_t log_level;

typedef int (*printf_like_t)(const char *, ...);

extern printf_like_t s_log_print_func;

char* timestr(void);

void log_set_level(log_level_t log_level_);// {log_level = log_level_;}

void log_set_printf(printf_like_t func);

char* log_cut_str(char* full_path, uint8_t len);

#define CONFIG_LOG_COLORS 1

#define TAG_DEFAULT		  "smh"

#if CONFIG_LOG_COLORS
#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D       LOG_COLOR(LOG_COLOR_BLUE)
#define LOG_COLOR_V
#else //CONFIG_LOG_COLORS
#define LOG_COLOR_E
#define LOG_COLOR_W
#define LOG_COLOR_I
#define LOG_COLOR_D
#define LOG_COLOR_V
#define LOG_RESET_COLOR
#endif //CONFIG_LOG_COLORS

#define FUNC_NAME_LEN	10
#define FILE_NAME_LEN	20

#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " (%s)(%10s)(..%12s)(%4d): " format LOG_RESET_COLOR "\n"

#define CONFIG_LOG_DEFAULT_LEVEL LOG_INFO
#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL  ((log_level_t) CONFIG_LOG_DEFAULT_LEVEL)
#endif

#define LOGE( format, ... )  if (log_level >= LOG_ERROR)   { s_log_print_func(LOG_FORMAT(E, format), timestr(), log_cut_str((char*)__FUNCTION__, FUNC_NAME_LEN), log_cut_str((char*)__FILE__, FILE_NAME_LEN), __LINE__, ##__VA_ARGS__); }
#define LOGW( format, ... )  if (log_level >= LOG_WARN)    { s_log_print_func(LOG_FORMAT(W, format), timestr(), log_cut_str((char*)__FUNCTION__, FUNC_NAME_LEN), log_cut_str((char*)__FILE__, FILE_NAME_LEN), __LINE__, ##__VA_ARGS__); }
#define LOGI( format, ... )  if (log_level >= LOG_INFO)    { s_log_print_func(LOG_FORMAT(I, format), timestr(), log_cut_str((char*)__FUNCTION__, FUNC_NAME_LEN), log_cut_str((char*)__FILE__, FILE_NAME_LEN), __LINE__, ##__VA_ARGS__); }
#define LOGD( format, ... )  if (log_level >= LOG_DEBUG)   { s_log_print_func(LOG_FORMAT(D, format), timestr(), log_cut_str((char*)__FUNCTION__, FUNC_NAME_LEN), log_cut_str((char*)__FILE__, FILE_NAME_LEN), __LINE__, ##__VA_ARGS__); }
#define LOGV( format, ... )  if (log_level >= LOG_VERBOSE) { s_log_print_func(LOG_FORMAT(V, format), timestr(), log_cut_str((char*)__FUNCTION__, FUNC_NAME_LEN), log_cut_str((char*)__FILE__, FILE_NAME_LEN), __LINE__, ##__VA_ARGS__); }

#ifdef __cplusplus
}
#endif

#endif /* LOG_H__ */
