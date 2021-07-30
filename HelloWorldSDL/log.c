#include <stdbool.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <fileapi.h>
#include "log.h"
#include "types.h"

static FILE* log_file;


static bool create_log_file(void)
{
	u32 i;
	char* log_file_name = malloc(sizeof(LOG_FILE) + (sizeof (u32) * 2));
	if (!log_file_name) {
		printf("ERROR: Could not set up log file\n");
		goto EXIT_ERROR;
	}

	sprintf(log_file_name, LOG_FILE);

	i = 0;
	// check if log file exists. If yes, move it
	if (0 == GetFileAttributesA(log_file_name)) {
		while (!GetFileAttributesA(log_file_name)) {
			sprintf(log_file_name, "%s%" PRIu32 "X", LOG_FILE, i);
			i++;
		}
		sprintf(log_file_name, "%s%" PRIu32 "X", LOG_FILE, i);

		if (!rename(LOG_FILE, log_file_name)) {
			printf("ERROR: Could not move log file!\n");
			goto EXIT_ERROR_CLEANUP;
		}
	}

	log_file = fopen(log_file_name, "w");
	free(log_file_name);
	return 1;

EXIT_ERROR_CLEANUP:
	free(log_file_name);
EXIT_ERROR:
	return 0;
}

void log_this(
	const char* severity,
	const char* time,
	const char* file,
	const char* func,
	long line,
	const char* fmt, ...)
{
	va_list args;

	va_start(args, fmt);

	if (!log_file) {
			create_log_file();
	}
	if (log_file) {
		fprintf(log_file, "%s %s %s %s():% 4ld: ", time, severity, file, func, line);
		vfprintf(log_file, fmt, args);
		fprintf(log_file, "\n");
	}
	printf("%s %s %s %s():% 4ld: ", time, severity, file, func, line);
	vprintf(fmt, args);
	printf("\n");
	va_end(args);
}