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


#ifndef _COMMON_H
#define _COMMON_H 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


typedef struct _result {
	struct _result	*next;
	char		*text;
} result_t;


typedef union {
	uint32_t	as_uint;
	char		as_char[4];
} length_t;


typedef struct _txt {
	struct _txt *next;
	char	text[1024];
} txt_t;


// Prototypes for config.c

void  config_read(char *google, char *mozilla, char *timeout, char *force);

char *config_get_google(void);
char *config_get_mozilla(void);
int   config_get_timeout(void);
int   config_get_force(void);


// Prototypes for avahi.c

result_t *avahi_browse(void);


// Prototypes for query.c

result_t *query_browse(void);


// Prototypes for install.c

void install_install(char *prog);
void install_uninstall(void);


// Prototypes for util.c

#define UTIL_STRCPY(dst, src)	util_strcpy(dst, src, sizeof(dst))
#define UTIL_STRCAT(dst, src)	util_strcat(dst, src, sizeof(dst))

void *util_malloc(size_t size);
void *util_realloc(void *ptr, size_t size, size_t prev);
char *util_strdup(const char *str);
void  util_free(void *ptr);

char *util_strcpy(char *dst, const char *src, size_t len);
char *util_strcat(char *dst, const char *src, size_t len);
char *util_strtrim(char *src, const char *trim);
char *util_append(char *dst, size_t len, char *fmt, ...);

void util_inc_verbose(void);
int  util_get_verbose(void);

void util_open_logfile(char *logfile);
void util_debug(int level, char *msg, ...);
void util_info(char *msg, ...);
void util_error(const char *func, int line, char *msg, ...);
void util_fatal(char *msg, ...);

#endif /* !_COMMON_H */

