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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <syslog.h>
#include <time.h>
#include <errno.h>
#include <err.h>
#include <unistd.h>
#include <regex.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>


typedef struct server_t {
	struct server_t	*next;
	char		*text;
} result_t;


typedef union {
	unsigned int as_uint;
	char         as_char[4];
} length_t;


// Prototypes for avahi.c

int avahi_browse(void);
result_t *avahi_get_result(void);
void avahi_init(void);


// Prototypes for mdnssd.c

int mdnssd_browse(void);
result_t *mdnssd_get_result(void);
void mdnssd_init(void);


// Prototypes for options.c

int options_get_timeout(void);
int options_use_avahi(void);
int options_use_debug(void);
void options_init(char *argv0);


// Prototypes for util.c

#define UTIL_STRCPY(dst, src)	util_strcpy(dst, src, sizeof(dst))
#define UTIL_STRCAT(dst, src)	util_strcat(dst, src, sizeof(dst))

void *util_malloc(size_t size);
void *util_realloc(void *ptr, size_t size, size_t prev);
char *util_strdup(char *str);
void util_free(void *ptr);

char *util_strcpy(char *dst, char *src, size_t len);
char *util_strcat(char *dst, char *src, size_t len);
char *util_append(char *dst, size_t len, char *fmt, ...);

void util_verbose(void);
void util_set_log(char *ident);
void util_debug(char *msg, ...);
void util_info(char *msg, ...);
void util_warn(char *msg, ...);
void util_error(char *msg, ...);
void util_fatal(char *msg, ...);

