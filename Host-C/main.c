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

#include <getopt.h>


static struct option long_options[] = {
	{ "help",    no_argument, NULL, 'h' },
	{ "json",    no_argument, NULL, 'j' },
	{ "verbose", no_argument, NULL, 'v' },
	{ NULL, 0, NULL, 0 }
};


static int json = 0;


static void
usage(char *name, int retval)
{
	FILE *fp = (retval == EXIT_SUCCESS ? stdout : stderr);

	fprintf(fp, "Syntax: %s [options ...]\n", name);
	fprintf(fp, "   -h|--help      Help (show this text) and exit\n");
	fprintf(fp, "   -j|--json      Print human readable length prolog\n");
	fprintf(fp, "   -v|--verbose   Send logs to stderr besides syslog\n");

	exit(retval);
}


static void
await_command(void)
{
	char buffer[1024], temp;
	size_t ofs_l, ofs_m;
	struct timeval timer;
	fd_set rfds, wfds, xfds;
	int hfd, res;
	length_t length;

	for (ofs_l = 0; ; ) {
		timer.tv_sec  = 1;
		timer.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		FD_ZERO(&xfds);

		FD_SET(fileno(stdin), &rfds);
		hfd = fileno(stdin);

		if ((res = select(hfd + 1, &rfds, &wfds, &xfds, &timer)) == -1) {
			util_fatal("can't select-await (%s)", strerror(errno));
		}
		if (res == 0) {
			util_debug("tick-await...");
			continue;
		}

		if (FD_ISSET(fileno(stdin), &rfds)) {
			read(fileno(stdin), &temp, 1);
			if (ofs_l < sizeof(length.as_uint)) {
				memset(buffer, '\0', sizeof(buffer));
				ofs_m = 0;
				length.as_char[ofs_l++] = temp;
				continue;
			}
			if (length.as_uint >= sizeof(buffer)) {
				util_error("length %u bigger than buffer size %u",
						length.as_uint, sizeof(buffer));
				ofs_l = 0;
			}

			if (ofs_m < length.as_uint) {
				if (temp > 0x20 && temp < 0x7f) {
					util_debug("Got stdin (%2u): %c",   ofs_m, temp);
				} else {
					util_debug("Got stdin (%2u): %02x", ofs_m, temp);
				}
				buffer[ofs_m] = temp;
			}
			if (++ofs_m == length.as_uint) {
				util_info("message: '%s'", buffer);
				return;
			}
		}
	}
}


static void
send_result(char *source, result_t *result)
{
	char prolog[1024], *epilog;
	length_t length;
	result_t *runner;

	UTIL_STRCPY(prolog, "{\n  \"version\": 2,\n");
	util_append(prolog, sizeof(prolog), "  \"source\": \"%s\",\n", source);
	UTIL_STRCAT(prolog, "  \"result\": [\n");
	epilog = "  ]\n}\n";
	length.as_uint = strlen(prolog) + strlen(epilog);

	for (runner = result; runner != NULL; runner = runner->next) {
		length.as_uint += strlen(runner->text) + 1;
		if (runner->next != NULL) {
			length.as_uint += 1;
		}
	}

	if (json == 0) {
		write(fileno(stdout), length.as_char, 4);
	} else {
		printf("%u bytes ==>\n", length.as_uint);
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
	int c, ofs, query;
	length_t length;
	time_t time_up;
	result_t *result;

	if (sizeof(length.as_uint) != 4) {
		errx(EXIT_FAILURE, "sizeof(length) is %lu, not 4", sizeof(length.as_uint));
	}

	for (ofs = 0; ; ) {
		c = getopt_long(argc, argv, "h?jv", long_options, &ofs);
		if (c < 0) {
			break;
		}
		switch (c) {
			case 'h':
			case '?':
				usage(argv[0], EXIT_SUCCESS);
				break;
			case 'j':
				json = 1;
				break;
			case 'v':
				util_verbose();
				break;
			default:
				usage(argv[0], EXIT_FAILURE);
				break;
		}
	}

	config_init(argv[0]);
	util_set_log("zeroconf_lookup");
	avahi_init();
	mdnssd_init();

	if (json == 0) {
		await_command();
	}

	if (avahi_browse() == 0) {
		send_result("avahi", avahi_get_result());
		exit(EXIT_SUCCESS);
	}

	mdnssd_browse();
	send_result("mDNS-SD", mdnssd_get_result());
	exit(EXIT_SUCCESS);
}

