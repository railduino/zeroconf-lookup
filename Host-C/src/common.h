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

#define HOST_NAME	"com.railduino.zeroconf_lookup"
#define DESCRIPTION	"Find HTTP Servers in the .local domain using Zeroconf"
#define CHROME_TAG	"anjclddigfkhclmgopnjmmpfllfbhfea"
#define MOZILLA_TAG	"zeroconf_lookup@railduino.com"
#define TIME_OUT	2
#define DEFAULT_LOGFILE	"/tmp/zeroconf_lookup.log"


#include "config.h"

#if defined(HAVE_WINSOCK2_H)
#  include <winsock2.h>		// includes windows.h
#  pragma comment(lib, "Ws2_32.lib")
#endif
#if defined(HAVE_WS2TCPIP_H)
#  include <ws2tcpip.h>
#endif
#if defined(HAVE_IO_H)
#  include <io.h>
#endif

#if defined(HAVE_STDIO_H)
#  include <stdio.h>
#endif
#if defined(HAVE_STDDEF_H)
#  include <stddef.h>
#endif
#if defined(HAVE_STDLIB_H)
#  include <stdlib.h>
#endif
#if defined(HAVE_STDARG_H)
#  include <stdarg.h>
#endif
#if defined(HAVE_STRING_H)
#  include <string.h>
#endif
#if defined(HAVE_TIME_H)
#  include <time.h>
#endif
#if defined(HAVE_ERRNO_H)
#  include <errno.h>
#endif
#if defined(HAVE_STDINT_H)
#  include <stdint.h>
#endif
#if defined(HAVE_SYS_TYPES_H)
#  include <sys/types.h>
#endif
#if defined(HAVE_UNISTD_H)
#  include <unistd.h>
#endif
#if defined(HAVE_FCNTL_H)
#  include <fcntl.h>
#endif
#if defined(HAVE_SYS_STAT_H)
#  include <sys/stat.h>
#endif
#if defined(HAVE_GETOPT_H)
#  include <getopt.h>
#endif
#if defined(HAVE_SYSLOG_H)
#  include <syslog.h>
#endif
#if defined(HAVE_POLL_H)
#  include <poll.h>
#endif
#if defined(HAVE_SYS_POLL_H)
#  include <sys/poll.h>
#endif

#if defined(HAVE_NETDB_H)
#  include <netdb.h>
#endif
#if defined(HAVE_SOCKET_H)
#  include <socket.h>
#endif
#if defined(HAVE_SYS_SOCKET_H)
#  include <sys/socket.h>
#endif
#if defined(HAVE_NETINET_IN_H)
#  include <netinet/in.h>
#endif
#if defined(HAVE_ARPA_INET_H)
#  include <arpa/inet.h>
#endif

#if defined(HAVE_MACH_O_DYLD_H)
#  include <mach-o/dyld.h>
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
	char	text[1024];
} txt_t;


// Prototypes for avahi.c (Linux only)

#if defined(AVAHI_FOUND)
result_t *avahi_browse(void);
#endif


// Prototypes for dnssd.c (mDNSResponder)

#if defined(DNSSD_FOUND)
result_t *dnssd_browse(void);
#endif


// Prototypes for query.c (built-in mDNS-SD)

#if defined(QUERY_FOUND)
result_t *query_browse(void);
#endif


// Prototypes for empty.c (if nothing else worked)

result_t *empty_browse(void);


// Prototypes for options.c

char *options_get_string(char *name, char *dflt);
int options_get_number(char *name, int dflt, int min, int max);
void options_init(void);


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

