/****************************************************************************
 *
 * Copyright (c) 2017-2018 Volker Wiegand <volker@railduino.de>
 *
 * This file is part of Zeroconf-Lookup.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ****************************************************************************/

#include "common.h"


static int verbose = 0;


void *
util_malloc(size_t size)
{
	void *ptr;

	if ((ptr = calloc(1, size)) == NULL) {
		util_fatal("out of memory");
	}

	return ptr;
}


void *
util_realloc(void *ptr, size_t size, size_t prev)
{
	if ((ptr = realloc(ptr, size)) == NULL) {
		util_fatal("out of memory");
	}

	if (size > prev) {
		memset(ptr + prev, '\0', size - prev);
	}

	return ptr;
}


char *
util_strdup(char *str)
{
	char *ptr;
	size_t len;

	if (str == NULL) {
		return NULL;
	}

	len = strlen(str);
	ptr = util_malloc(len + 1);
	strncpy(ptr, str, len);

	return ptr;
}


void
util_free(void *ptr)
{
	if (ptr) {
		free(ptr);
	}
}


char *
util_strcpy(char *dst, char *src, size_t len)
{
	if (dst == NULL || src == NULL) {
		return NULL;
	}

	memset(dst, 0, len);
	strncpy(dst, src, len - 1);

	return dst;
}


char *
util_strcat(char *dst, char *src, size_t len)
{
	size_t siz;

	if (dst == NULL || src == NULL)
		return NULL;
	siz = strlen(dst);

	memset(dst + siz, 0, len - siz);
	strncpy(dst + siz, src, len - siz - 1);

	return dst;
}


char *
util_append(char *dst, size_t len, char *fmt, ...)
{
	char tmp[FILENAME_MAX];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(tmp, sizeof(tmp), fmt, ap);
	va_end(ap);

	return util_strcat(dst, tmp, len);
}


void
util_verbose(void)
{
	verbose++;
}


static void
util_close_log(void)
{
	syslog(LOG_INFO, "Program terminated");
	closelog();
}


void
util_set_log(char *ident)
{
	openlog(ident, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
	atexit(util_close_log);

	if (options_use_debug() == 0) {
		setlogmask(LOG_UPTO(LOG_INFO));
	}

	syslog(LOG_INFO, "Program started by User %d", getuid());
}


void
util_debug(char *fmt, ...)
{
	char buffer[1000];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	syslog(LOG_DEBUG, "%s", buffer);
	if (verbose) {
		fprintf(stderr, "DEBUG: %s\n", buffer);
	}
}


void
util_info(char *fmt, ...)
{
	char buffer[1000];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	syslog(LOG_INFO, "%s", buffer);
	if (verbose) {
		fprintf(stderr, "INFO:  %s\n", buffer);
	}
}


void
util_warn(char *fmt, ...)
{
	char buffer[1000];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	syslog(LOG_WARNING, "%s", buffer);
	if (verbose) {
		fprintf(stderr, "WARN:  %s\n", buffer);
	}
}


void
util_error(char *fmt, ...)
{
	char buffer[1000];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	syslog(LOG_ERR, "%s", buffer);
	if (verbose) {
		fprintf(stderr, "ERROR: %s\n", buffer);
	}
}


void
util_fatal(char *fmt, ...)
{
	char buffer[1000];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	syslog(LOG_CRIT, "%s (exiting)", buffer);
	fprintf(stderr, "FATAL: %s\n", buffer);

	exit(EXIT_FAILURE);
}

