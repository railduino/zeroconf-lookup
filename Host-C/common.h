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

#define VERSION		"2.0.0"
#define HOST_NAME	"com.railduino.zeroconf_lookup"
#define DESCRIPTION	"Find HTTP Servers in the .local domain using Zeroconf"
#define CHROME_TAG	"anjclddigfkhclmgopnjmmpfllfbhfea"
#define MOZILLA_TAG	"zeroconf_lookup@railduino.com"
#define TIME_OUT	2


#if defined(__STDC__)
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <string.h>
#  include <time.h>
#  include <errno.h>
#endif

#if defined(__linux__) || defined(__APPLE__)
#  include <unistd.h>
#  include <fcntl.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <stdint.h>
#  define HAVE_POLL 1
#  include <poll.h>
#  define HAVE_GETOPT_LONG 1
#  include <getopt.h>
#  define DEFAULT_LOGFILE "/tmp/zeroconf_lookup.log"
#  if defined(__linux__)
#    define HAVE_AVAHI 1
#  endif
#  if defined(__APPLE__)
#    include <mach-o/dyld.h>
#    define HAVE_DNSSD 1
#  endif
#  define HAVE_QUERY 1
#endif

#if defined(_WIN32)
#  include <Windows.h>
#  define HAVE_DNSSD 1
#endif


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
	char	*text;
} txt_t;


// Prototypes for avahi.c (Linux only)

#if defined(HAVE_AVAHI)
result_t *avahi_browse(void);
#endif


// Prototypes for dnssd.c (mDNSResponder)

#if defined(HAVE_DNSSD)
result_t *dnssd_browse(void);
#endif


// Prototypes for query.c (built-in mDNS-SD)

#if defined(HAVE_QUERY)
result_t *query_browse(void);
#endif


// Prototypes for empty.c (if nothing else worked)

result_t *empty_browse(void);


// Prototypes for options.c

char *options_get_string(char *name, char *dflt);
int options_get_number(char *name, int dflt, int min, int max);
void options_init(char *argv0);


// Prototypes for install.c

void install_set_chrome(char *str);
void install_set_mozilla(char *str);
void install_set_timeout(char *str);

void install_install(char *prog);
void install_uninstall(void);


// Prototypes for util.c

#define UTIL_STRCPY(dst, src)	util_strcpy(dst, src, sizeof(dst))
#define UTIL_STRCAT(dst, src)	util_strcat(dst, src, sizeof(dst))

void *util_malloc(size_t size);
void *util_realloc(void *ptr, size_t size, size_t prev);
char *util_strdup(const char *str);
void util_free(void *ptr);

char *util_strcpy(char *dst, const char *src, size_t len);
char *util_strcat(char *dst, const char *src, size_t len);
char *util_strtrim(char *src, const char *trim);
char *util_append(char *dst, size_t len, char *fmt, ...);

void util_inc_verbose(void);
int  util_get_verbose(void);
void util_open_logfile(char *logfile);
void util_debug(int level, char *msg, ...);
void util_info(char *msg, ...);
void util_warn(char *msg, ...);
void util_error(char *msg, ...);
void util_fatal(char *msg, ...);

