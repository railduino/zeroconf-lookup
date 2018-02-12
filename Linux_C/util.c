/****************************************************************************
 *
 * Copyright (c) 2017-2018 Volker Wiegand <volker@railduino.de>
 *
 * This file is part of Zeroconf-Lookup.
 * Project home: https://www.railduino.de/zeroconf-lookup
 * Source code:  https://github.com/railduino/zeroconf-lookup.git
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

#include <time.h>


static int   my_verbose = 0;
static FILE *my_log_fp  = NULL;


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
		memset((char *) ptr + prev, '\0', size - prev);
	}

	return ptr;
}


char *
util_strdup(const char *str)
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
	if (ptr != NULL) {
		free(ptr);
	}
}


char *
util_strcpy(char *dst, const char *src, size_t len)
{
	if (dst == NULL || src == NULL) {
		return NULL;
	}

	memset(dst, 0, len);
	strncpy(dst, src, len - 1);

	return dst;
}


char *
util_strcat(char *dst, const char *src, size_t len)
{
	size_t siz;

	if (dst == NULL || src == NULL) {
		return NULL;
	}

	siz = strlen(dst);

	memset(dst + siz, 0, len - siz);
	strncpy(dst + siz, src, len - siz - 1);

	return dst;
}


char *
util_strtrim(char *src, const char *trim)
{
	size_t siz;

	if (src == NULL) {
		return NULL;
	}
	if (trim == NULL) {
		trim = " \t\n\r";
	}

	while (strlen(src) > 0 && strchr(trim, *src) != NULL) {
		memmove(src, src + 1, strlen(src));
	}

	while ((siz = strlen(src)) > 0) {
		if (strchr(trim, src[siz-1]) != NULL) {
			src[siz-1] = '\0';
		} else {
			break;
		}
	}

	return src;
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
util_inc_verbose(void)
{
	my_verbose++;
}


int
util_get_verbose(void)
{
	return my_verbose;
}


static void
util_close_logfile(void)
{
	if (my_log_fp != NULL) {
		util_info("touch-down");
		fclose(my_log_fp);
		my_log_fp = NULL;
	}
}


void
util_open_logfile(char *logfile)
{
	if ((my_log_fp = fopen(logfile, "w")) == NULL) {
		util_fatal("can't create logfile %s: %s", logfile, strerror(errno));
	}
	atexit(util_close_logfile);

	util_info("take-off verbose=%d", my_verbose);
}


static void
util_write_logfile(char *tag, char *line)
{
	time_t now = time(NULL);

	if (my_log_fp != NULL) {
		fprintf(my_log_fp, "%.19s %s -- %s\n", ctime(&now), tag, line);
	}
}


void
util_debug(int level, char *fmt, ...)
{
	char buffer[1000];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	if (level > my_verbose) {
		return;
	}

	if (my_log_fp != NULL) {
		time_t now = time(NULL);
		fprintf(my_log_fp, "%.19s DEBUG -- %s\n", ctime(&now), buffer);
	} else {
		fprintf(stderr, "DEBUG - %s\n", buffer);
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

	if (my_log_fp != NULL) {
		time_t now = time(NULL);
		fprintf(my_log_fp, "%.19s INFO  -- %s\n", ctime(&now), buffer);
	} else {
		fprintf(stderr, "INFO - %s\n", buffer);
	}
}


void
util_error(const char *func, int line, char *fmt, ...)
{
	char buffer[1000];
	size_t ofs;
	va_list ap;

	snprintf(buffer, sizeof(buffer), ">>>%s:%d<<< ", func, line);
	ofs = strlen(buffer);

	va_start(ap, fmt);
	vsnprintf(buffer + ofs, sizeof(buffer) - ofs, fmt, ap);
	va_end(ap);

	if (my_log_fp != NULL) {
		time_t now = time(NULL);
		fprintf(my_log_fp, "%.19s ERROR -- %s\n", ctime(&now), buffer);
	} else {
		fprintf(stderr, "ERROR - %s\n", buffer);
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

	util_write_logfile("FATAL", buffer);
	if (my_log_fp != NULL) {
		time_t now = time(NULL);
		fprintf(my_log_fp, "%.19s ERROR -- %s\n", ctime(&now), buffer);
	}
	fprintf(stderr, "FATAL - %s\n", buffer);

	exit(EXIT_FAILURE);
}

