#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

#ifndef LOG_FILE
#define LOG_FILE "./logs/log"
#endif

#define LOG_WARNING(fmt, ...) do { \
		log_this("WARNING", __TIME__, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__); \
	} while(0)

#if defined DEBUG || defined _DEBUG
	#define LOG_DEBUG(fmt, ...) do { \
			log_this("DEBUG  ", __TIME__, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__); \
		} while(0)
#else
	#define LOG_DEBUG(fmt, ...) do { \
		} while(0)
#endif

#define LOG_INFO(fmt, ...) do { \
		log_this("INFO   ", __TIME__, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__); \
	} while(0)

#define LOG_ERROR(fmt, ...) do { \
		log_this("ERROR  ", __TIME__, __FILE__, __func__, __LINE__, fmt, __VA_ARGS__); \
	} while(0)


void log_this(
	const char *severity,
	const char *time,
	const char *file,
	const char *func,
	long line,
	const char *fmt, ...);

#endif