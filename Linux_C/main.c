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
#include "config.h"

#include <getopt.h>
#include <poll.h>


#define VERSION		"2.4.0"
#define LOG_FILE	"/tmp/zeroconf_lookup.log"


static struct option long_options[] = {
	{ "force",     required_argument, NULL, 'f' },
	{ "google",    required_argument, NULL, 'g' },
	{ "help",      no_argument,       NULL, 'h' },
	{ "install",   no_argument,       NULL, 'i' },
	{ "mozilla",   required_argument, NULL, 'm' },
	{ "log",       no_argument,       NULL, 'l' },
	{ "readable",  no_argument,       NULL, 'r' },
	{ "timeout",   required_argument, NULL, 't' },
	{ "uninstall", no_argument,       NULL, 'u' },
	{ "verbose",   no_argument,       NULL, 'v' },
	{ NULL, 0, NULL, 0 }
};

static char     my_input[64];
static length_t my_length;
static size_t   my_length_offset = 0;
static size_t   my_input_offset;


static void
main_usage(char *name, int retval)
{
	FILE *fp = (retval == EXIT_SUCCESS ? stdout : stderr);

	fprintf(fp, "\n");
	fprintf(fp, "Railduino zeroconf_lookup Version %s\n", VERSION);
	fprintf(fp, "Usage: %s [options ...]\n", name);
	fprintf(fp, "      -f|--force=<avahi|query>   Enforce query method\n");
	fprintf(fp, "                                     Default: empty (use avahi if available, else query)\n");
	fprintf(fp, "      -g|--google=<tag>          Change Google Chrome/Chromium allowed_origins\n");
	fprintf(fp, "                                     Default: %s\n", GOOGLE_TAG);
	fprintf(fp, "      -h|--help                  Display this usage information and exit\n");
	fprintf(fp, "      -i|--install               Install Firefox/Chrome manifests (sudo for system wide)\n");
	fprintf(fp, "      -m|--mozilla=<tag>         Change Mozilla Firefox allowed_extensions\n");
	fprintf(fp, "                                     Default: %s\n", MOZILLA_TAG);
	fprintf(fp, "      -l|--log                   Write logfile (%s)\n", LOG_FILE);
	fprintf(fp, "      -r|--readable              Use human readable length for output\n");
	fprintf(fp, "      -t|--timeout=<num>         Set query timeout\n");
	fprintf(fp, "                                     Default: %s sec)\n", TIME_OUT);
	fprintf(fp, "      -u|--uninstall             Uninstall Firefox/Chrome manifests (sudo for system wide)\n");
	fprintf(fp, "      -v|--verbose               Increase verbosity level\n");
	fprintf(fp, "\n");

	exit(retval);
}


static int
main_input_byte(char chr)
{
	if (my_length_offset < sizeof(my_length.as_uint)) {
		util_debug(1, "got length byte %d = %02x", my_length_offset, (int) chr & 0xff);
		memset(my_input, '\0', sizeof(my_input));
		my_input_offset = 0;
		my_length.as_char[my_length_offset++] = chr;
		return 0;
	}

	if (my_length.as_uint >= sizeof(my_input)) {
		util_error(__func__, __LINE__, "length %u bigger than buffer size %u",
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
main_receive_input(void)
{
	util_debug(1, "awaiting input (poll), 5 sec max");
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
			if (main_input_byte(temp) == 1) {
				return;
			}
		}
	}
}


static void
main_send_result(char *source, int readable, result_t *result)
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
	static char avahi[256], query[256], google[256], mozilla[256], timeout[32], force[32];
	int c, do_log, readable, do_inst, do_uninst;
	result_t *result;

	snprintf(avahi, sizeof(avahi), "Avahi (C, %s)", VERSION);
	snprintf(query, sizeof(query), "Query (C, %s)", VERSION);

	do_log = readable = do_inst = do_uninst = 0;
	for (;;) {
		c = getopt_long(argc, argv, "f:g:h?im:lrt:uv", long_options, NULL);
		if (c < 0) {
			break;
		}

		switch (c) {
			case 'f':
				UTIL_STRCPY(force, optarg);
				break;
			case 'g':
				UTIL_STRCPY(google, optarg);
				break;
			case 'h':
			case '?':
				main_usage(argv[0], EXIT_SUCCESS);
				break;
			case 'i':
				do_inst = 1;
				break;
			case 'm':
				UTIL_STRCPY(mozilla, optarg);
				break;
			case 'l':
				do_log = 1;
				break;
			case 'r':
				readable = 1;
				break;
			case 't':
				UTIL_STRCPY(timeout, optarg);
				break;
			case 'u':
				do_uninst = 1;
				break;
			case 'v':
				util_inc_verbose();
				break;
			default:
				main_usage(argv[0], EXIT_FAILURE);
				break;
		}
	}

	if (do_log == 0 && do_inst == 0 && do_uninst == 0) {
		util_open_logfile(LOG_FILE);
	}

	config_read(google, mozilla, timeout, force);

	if (do_inst == 1 || do_uninst == 1) {
		if (do_uninst == 1) {
			install_uninstall();
		}
		if (do_inst == 1) {
			install_install(argv[0]);
		}
		exit(EXIT_SUCCESS);
	}

	if (readable == 0) {
		main_receive_input();
	}

	if (strcmp(config_get_force(), "avahi") == 0) {
		main_send_result(avahi, readable, avahi_browse());
		exit(EXIT_SUCCESS);
	}
	if (strcmp(config_get_force(), "query") == 0) {
		main_send_result(query, readable, query_browse());
		exit(EXIT_SUCCESS);
	}

	if ((result = avahi_browse()) != NULL) {
		main_send_result(avahi, readable, result);
		exit(EXIT_SUCCESS);
	}
	if ((result = query_browse()) != NULL) {
		main_send_result(query, readable, result);
		exit(EXIT_SUCCESS);
	}

	main_send_result(avahi, readable, NULL);
	exit(EXIT_SUCCESS);
}

