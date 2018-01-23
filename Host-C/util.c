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


static int   my_verbose = 0;
static FILE *my_logfile = NULL;


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
		trim = " \t\n";
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
	if (my_logfile != NULL) {
		if (my_logfile != stderr) {
			util_info("touch-down");
			fclose(my_logfile);
		}
		my_logfile = NULL;
	}
}


void
util_open_logfile(char *logfile)
{
	if (logfile == NULL || strcmp("none", logfile) == 0) {
		return;
	}
	atexit(util_close_logfile);

	if (strcmp("stderr", logfile) == 0) {
		my_logfile = stderr;
	} else {
		if ((my_logfile = fopen(logfile, "w")) == NULL) {
			util_fatal("can't create logfile %s: %s", logfile, strerror(errno));
		}
		util_info("take-off logging=%d", my_verbose);
	}
}


static void
util_write_logfile(int tag, char *line)
{
	time_t now = time(NULL);
	char   *typ;

	if (my_logfile != NULL) {
		switch (tag) {
			case 'D': typ = "DEBUG"; break;
			case 'I': typ = "INFO "; break;
			case 'W': typ = "WARN "; break;
			case 'E': typ = "ERROR"; break;
			case 'F': typ = "FATAL"; break;
			default: return;
		}

		fprintf(my_logfile, "%.19s %s -- %s\n", ctime(&now), typ, line);
		fflush(my_logfile);
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

	if (level <= my_verbose) {
		util_write_logfile('D', buffer);
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

	util_write_logfile('I', buffer);
}


void
util_warn(char *fmt, ...)
{
	char buffer[1000];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	util_write_logfile('W', buffer);
}


void
util_error(char *fmt, ...)
{
	char buffer[1000];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	util_write_logfile('E', buffer);
}


void
util_fatal(char *fmt, ...)
{
	char buffer[1000];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	util_write_logfile('F', buffer);
	exit(EXIT_FAILURE);
}

