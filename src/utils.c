#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "utils.h"

void logprint(char *fmt, ...) {
	time_t now = time(NULL);
	struct tm *time = localtime(&now);
	va_list args;

	char timefmt[26]; // yyyy-mm-dd hh:mm:ss +hhmm
	strftime(timefmt, sizeof(timefmt) / sizeof(char), "%F %T %z", time);

	printf("[%s] ", timefmt);

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	putchar('\n');
}
