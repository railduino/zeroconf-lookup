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

#define VERSION		"1.0.0"

#if defined(__unix__)
#  include <getopt.h>
#  include <poll.h>
#  if defined(__APPLE__)
#    define DEFAULT_LOGFILE	"/tmp/zeroconf_lookup.log"
#  else
#    define DEFAULT_LOGFILE	"/tmp/zeroconf_lookup.log"
#  endif
#elif defined(_WIN32)
#  include <Windows.h>
#    define DEFAULT_LOGFILE	"use mktemp()"
#endif


static struct option long_options[] = {
	{ "avahi",   no_argument,       NULL, 'a' },
	{ "dnssd",   no_argument,       NULL, 'd' },
	{ "empty",   no_argument,       NULL, 'e' },
	{ "help",    no_argument,       NULL, 'h' },
	{ "json",    no_argument,       NULL, 'j' },
	{ "log",     required_argument, NULL, 'l' },
	{ "query",   no_argument,       NULL, 'q' },
	{ "verbose", no_argument,       NULL, 'v' },
	{ NULL, 0, NULL, 0 }
};

static char     my_input[64];
static length_t my_length;
static size_t   my_length_offset = 0;
static size_t   my_input_offset;


static void
usage(char *name, int retval)
{
	FILE *fp = (retval == EXIT_SUCCESS ? stdout : stderr);

	fprintf(fp, "Railduino zeroconf_lookup Version %s\n", VERSION);
	fprintf(fp, "Usage: %s [options ...]\n", name);
	fprintf(fp, "      -a|--avahi        Ignore Avahi interface altogether\n");
	fprintf(fp, "      -d|--dnssd        Ignore DNS-SD interface altogether\n");
	fprintf(fp, "      -e|--empty        Ignore Empty interface altogether\n");
	fprintf(fp, "      -h|--help         Display usage and exit\n");
	fprintf(fp, "      -j|--json         Use human readable length\n");
	fprintf(fp, "      -l|--log=<file>   Change logfile [default %s]\n", DEFAULT_LOGFILE);
	fprintf(fp, "                        Recognizes 'none' or 'stderr'\n");
	fprintf(fp, "      -q|--query        Ignore Query interface altogether\n");
	fprintf(fp, "      -v|--verbose      Increase verbosity level\n");

	exit(retval);
}


static int
handle_input_byte(char chr)
{
	if (my_length_offset < sizeof(my_length.as_uint)) {
		util_debug(1, "got length byte %d = %02x", my_length_offset, (int) chr & 0xff);
		memset(my_input, '\0', sizeof(my_input));
		my_input_offset = 0;
		my_length.as_char[my_length_offset++] = chr;
		return 0;
	}

	if (my_length.as_uint >= sizeof(my_input)) {
		util_error("length %u bigger than buffer size %u",
				my_length.as_uint, sizeof(my_input));
		my_length_offset = 0;
		return 0;
	}
	util_debug(1, "length is complete: %u", my_length.as_uint);

	if (my_input_offset < my_length.as_uint) {
		if (chr > 0x20 && chr < 0x7f) {
			util_debug(1, "got message byte %2u = %c",   my_input_offset, chr);
		} else {
			util_debug(1, "got message byte %2u = %02x", my_input_offset, (int) chr & 0xff);
		}
		my_input[my_input_offset] = chr;
	}

	if (++my_input_offset == my_length.as_uint) {
		util_info("input complete: '%s'", util_strtrim(my_input, '"'));
		return 1;
	}

	return 0;
}


static void
receive_input(void)
{
#if defined(__unix__)
	util_debug(1, "awaiting input (poll)");
	for (;;) {
		struct pollfd fds[1];
		int ret;
		char temp;

		fds[0].fd = STDIN_FILENO;
		fds[0].events = POLLIN;
		if ((ret = poll(fds, 1, 5000)) == -1) {
			util_fatal("can't poll stdin (%s)", strerror(errno));
		}
		if (ret == 0) {
			util_fatal("timeout on stdin");
		}

		if (fds[0].revents & POLLIN) {
			read(fileno(stdin), &temp, 1);
			if (handle_input_byte(temp) == 1) {
				return;
			}
		}
	}
#elif defined(_WIN32)
	/**
	 * See: https://stackoverflow.com/a/35060700
	 */
	HANDLE hStdin;
	int nTimeout, nRun;
	DWORD dwAvailable;
	unsigned int temp;

	util_debug(1, "awaiting input (PeekNamedPipe)");
	nTimeout = GetTickCount() + 5000;
	hStdin   = GetStdHandle(STD_INPUT_HANDLE);

	while (GetTickCount() < nTimeout) {
		dwAvailable = 0; 
		PeekNamedPipe(hStdin, NULL, 0, NULL, &dwAvailable, NULL);
		if (dwAvailable > 0) {
			while (dwAvailable > 0) {
				temp = getchar();
				if (handle_input_byte((char) temp) == 1) {
					return;
				}
			}
		} else {
			Sleep(10);
		}
	}

	util_fatal("timeout on stdin");
#else
	util_fatal("no method for stdin");
#endif
}


static void
send_result(char *source, int json, result_t *result)
{
	char prolog[1024], *epilog;
	length_t length;
	result_t *runner;

	util_info("source: %s", source);

	UTIL_STRCPY(prolog, "{\n  \"version\": 2,\n");
	util_append(prolog, sizeof(prolog), "  \"source\": \"%s\",\n", source);
	UTIL_STRCAT(prolog, "  \"result\": [\n");
	length.as_uint = strlen(prolog);

	for (runner = result; runner != NULL; runner = runner->next) {
		length.as_uint += strlen(runner->text) + 1;
		if (runner->next != NULL) {
			length.as_uint += 1;
		}
	}

	epilog = "  ]\n}\n";
	length.as_uint += strlen(epilog);

	if (json == 0) {
		write(fileno(stdout), length.as_char, 4);
	} else {
		printf("==> %u bytes <==\n", length.as_uint);
	}

	printf("%s", prolog);
	for (runner = result; runner != NULL; runner = runner->next) {
		printf("%s", runner->text);
		printf("%s", runner->next != NULL ? ",\n" : "\n");
	}
	printf("%s", epilog);

	fflush(stdout);
}


int
main(int argc, char *argv[])
{
	int c, ofs, json, avahi, dnssd, empty, query;
	char *logfile = DEFAULT_LOGFILE;

	for (ofs = json = 0, avahi = dnssd = empty = query = 1; ; ) {
		c = getopt_long(argc, argv, "adeh?jl:qv", long_options, &ofs);
		if (c < 0) {
			break;
		}
		switch (c) {
			case 'a':
				avahi = 0;
				break;
			case 'd':
				dnssd = 0;
				break;
			case 'e':
				empty = 0;
				break;
			case 'h':
			case '?':
				usage(argv[0], EXIT_SUCCESS);
				break;
			case 'j':
				json = 1;
				break;
			case 'l':
				logfile = optarg;
				break;
			case 'q':
				query = 0;
				break;
			case 'v':
				util_inc_verbose();
				break;
			default:
				usage(argv[0], EXIT_FAILURE);
				break;
		}
	}

	options_init(argv[0]);
	util_open_logfile(logfile);

	if (json == 0) {
		receive_input();
	}

	if (avahi && avahi_found()) {
		send_result("Avahi", json, avahi_browse());
		exit(EXIT_SUCCESS);
	}

	if (dnssd && dnssd_found()) {
		send_result("DNS-SD", json, dnssd_browse());
		exit(EXIT_SUCCESS);
	}

	if (query && query_found()) {
		send_result("Query", json, query_browse());
		exit(EXIT_SUCCESS);
	}

	if (empty && empty_found()) {
		send_result("Empty", json, empty_browse());
		exit(EXIT_SUCCESS);
	}

	util_fatal("no discovery method");
}

