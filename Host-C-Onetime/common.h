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


// Prototypes for mdnssd.c

void mdnssd_select(int json, fd_set *rfds, fd_set *wfds, fd_set *xfds);
void mdnssd_prepare(int query, int *hfd, fd_set *rfds, fd_set *wfds, fd_set *xfds);
result_t *mdnssd_get_result(void);
void mdnssd_init(void);


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
void util_set_log(char *ident, int debug);
void util_debug(char *msg, ...);
void util_info(char *msg, ...);
void util_warn(char *msg, ...);
void util_error(char *msg, ...);
void util_fatal(char *msg, ...);

int util_get_timeout(char *argv0, int value);

