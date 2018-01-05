/****************************************************************************
 *
 * Copyright (c) 2017 Volker Wiegand <volker@railduino.de>
 *
 * This file is part of Zeroconf-Lookup.
 *
 *   Zeroconf-Lookup is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Zeroconf-Lookup is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *   See the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with Zeroconf-Lookup.
 *   If not, see <http://www.gnu.org/licenses/>.
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
util_set_log(char *ident, int debug)
{
	openlog(ident, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
	atexit(util_close_log);

	if (debug == 0) {
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


int
util_get_timeout(char *argv0, int value)
{
	char config[FILENAME_MAX], *ptr;
	FILE *fp = NULL;
	int count, result;

	if ((ptr = getenv("ZEROCONF_LOOKUP")) != NULL) {
		fp = fopen(ptr, "r");
	}
	if (fp == NULL && (ptr = getenv("HOME")) != NULL) {
		UTIL_STRCPY(config, ptr);
		UTIL_STRCAT(config, "/zeroconf_lookup.conf");
		ptr = config;
		fp = fopen(ptr, "r");
	}
	if (fp == NULL) {
		ptr = "/etc/zeroconf_lookup.conf";
		fp = fopen(ptr, "r");
	}
	if (fp == NULL) {
		UTIL_STRCPY(config, argv0);
		UTIL_STRCAT(config, ".conf");
		ptr = config;
		fp = fopen(ptr, "r");
	}
	if (fp == NULL) {
		util_debug("timeout %d (no config)", value);
		return value;
	}

	count = fscanf(fp, "%d", &result);
	fclose(fp);

	if (count == 1 && result > 0 && result < 10) {
		util_debug("timeout %d (%s)", result, ptr);
		return result;
	}

	util_debug("timeout %d (bad config)", value);
	return value;
}

