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


#if defined(HAVE_GETOPT_LONG)
static struct option long_options[] = {
	{ "chrome",    required_argument, NULL, 'c' },
	{ "help",      no_argument,       NULL, 'h' },
	{ "install",   no_argument,       NULL, 'i' },
	{ "log",       required_argument, NULL, 'l' },
	{ "mozilla",   required_argument, NULL, 'm' },
	{ "readable",  no_argument,       NULL, 'r' },
	{ "timeout",   required_argument, NULL, 't' },
	{ "uninstall", no_argument,       NULL, 'u' },
	{ "verbose",   no_argument,       NULL, 'v' },
	{ NULL, 0, NULL, 0 }
};
#endif

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
	fprintf(fp, "      -c|--chrome=<str>    Install Chrome allowed_origins (with -i)\n");
	fprintf(fp, "                           Default: %s\n", CHROME_TAG);
	fprintf(fp, "      -h|--help            Display usage and exit\n");
	fprintf(fp, "      -i|--install         Install Mozilla/Chrome manifests (sudo for system wide)\n");
	fprintf(fp, "      -l|--log=<file>      Change logfile (default %s)\n", DEFAULT_LOGFILE);
	fprintf(fp, "                           Recognizes 'none' for silence or 'stderr'\n");
	fprintf(fp, "      -m|--mozilla=<str>   Install Mozilla allowed_extensions (with -i)\n");
	fprintf(fp, "                           Default: %s\n", MOZILLA_TAG);
	fprintf(fp, "      -r|--readable        Use human readable length\n");
	fprintf(fp, "      -t|--timeout=<num>   Install server collect timeout (with -i)\n");
	fprintf(fp, "                           Default: %d, Range: 1 to 9\n", TIME_OUT);
	fprintf(fp, "      -u|--uninstall       Uninstall Mozilla/Chrome manifests (sudo for system wide)\n");
	fprintf(fp, "      -v|--verbose         Increase verbosity level\n");

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
		util_info("input complete: '%s'", util_strtrim(my_input, "\""));
		return 1;
	}

	return 0;
}


static void
receive_input(void)
{
#if defined(HAVE_POLL)
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
send_result(char *source, int readable, result_t *result)
{
	char prolog[1024], *epilog;
	length_t length;
	result_t *runner;

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

	if (readable == 0) {
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
	int c, ofs, readable, instmode;
	char *logfile = DEFAULT_LOGFILE;

#if defined(_WIN32)
	WSADATA wsaData;
	if ((c = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0) {
		util_fatal("WSAStartup failed: %d", c);
	}
	atexit(WSACleanup);
#endif

	for (ofs = readable = instmode = 0; ; ) {
#if defined(HAVE_GETOPT_LONG)
		c = getopt_long(argc, argv, "c:h?il:m:rt:uv", long_options, &ofs);
#else
		c = getopt(argc, argv, "c:h?il:m:rt:uv");
#endif
		if (c < 0) {
			break;
		}
		switch (c) {
			case 'c':
				install_set_chrome(optarg);
				break;
			case 'h':
			case '?':
				usage(argv[0], EXIT_SUCCESS);
				break;
			case 'i':
				instmode = 'i';
				break;
			case 'l':
				logfile = optarg;
				break;
			case 'm':
				install_set_mozilla(optarg);
				break;
			case 'r':
				readable = 1;
				break;
			case 't':
				install_set_timeout(optarg);
				break;
			case 'u':
				instmode = 'u';
				break;
			case 'v':
				util_inc_verbose();
				break;
			default:
				usage(argv[0], EXIT_FAILURE);
				break;
		}
	}

	if (instmode == 'i') {
		util_open_logfile("stderr");
		install_install(argv[0]);
		exit(EXIT_SUCCESS);
	}
	if (instmode == 'u') {
		util_open_logfile("stderr");
		install_uninstall();
		exit(EXIT_SUCCESS);
	}

	options_init(argv[0]);
	util_open_logfile(logfile);

	if (readable == 0) {
		receive_input();
	}
	util_debug(1, "ready for browsing");

#if defined(HAVE_AVAHI)
	if (options_get_number("Avahi", 1, 0, 1) == 1) {
		send_result("C (Avahi)", readable, avahi_browse());
		exit(EXIT_SUCCESS);
	}
#endif

#if defined(HAVE_DNSSD)
	if (options_get_number("mDNSResponder", 1, 0, 1) == 1) {
		send_result("C (mDNSResponder)", readable, dnssd_browse());
		exit(EXIT_SUCCESS);
	}
#endif

#if defined(HAVE_QUERY)
	if (options_get_number("Query", 1, 0, 1) == 1) {
		send_result("C (Query)", readable, query_browse());
		exit(EXIT_SUCCESS);
	}
#endif

	send_result("C (no Input)", readable, empty_browse());
	exit(EXIT_SUCCESS);
}

