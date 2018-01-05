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

#include "common.h"

#include <getopt.h>
#include <signal.h>


#define DEFAULT_TIMEOUT		3


static struct option long_options[] = {
	{ "debug",   no_argument, NULL, 'd' },
	{ "help",    no_argument, NULL, 'h' },
	{ "json",    no_argument, NULL, 'j' },
	{ "verbose", no_argument, NULL, 'v' },
	{ NULL, 0, NULL, 0 }
};


static void
usage(char *name, int retval)
{
	FILE *fp = (retval == EXIT_SUCCESS ? stdout : stderr);

	fprintf(fp, "Syntax: %s [options ...]\n", name);
	fprintf(fp, "   -d|--debug     Send debug to syslog(3) LOG_USER\n");
	fprintf(fp, "   -h|--help      Help (show this text) and exit\n");
	fprintf(fp, "   -j|--json      Print human readable length prolog\n");
	fprintf(fp, "   -v|--verbose   Send logs to stderr besides syslog\n");

	exit(retval);
}


static void
sighandler(int signum) {
	util_info("received signal %d", signum);
	exit(EXIT_SUCCESS);
}


int
main(int argc, char *argv[])
{
	int c, ofs, json, debug, query, timeout;
	char buffer[256], temp, *prolog, *epilog;
	union {
		unsigned int as_uint;
		char         as_char[4];
	} length;
	size_t ofs_l, ofs_m;
	time_t time_up;
	result_t *result;

	if (sizeof(length.as_uint) != 4) {
		errx(EXIT_FAILURE, "sizeof(length) is %lu, not 4", sizeof(length.as_uint));
	}

	for (ofs = json = debug = 0; ; ) {
		c = getopt_long(argc, argv, "dh?jv", long_options, &ofs);
		if (c < 0) {
			break;
		}
		switch (c) {
			case 'd':
				debug = 1;
				break;
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

	util_set_log("zeroconf_lookup", debug);

	signal(SIGTERM, sighandler);
	signal(SIGINT,  sighandler);

	mdnssd_init();

	timeout = util_get_timeout(argv[0], DEFAULT_TIMEOUT);

	for (query = 0, ofs_l = 0, time_up = 0; ; ) {
		struct timeval timer;
		fd_set rfds, wfds, xfds;
		int hfd;

		if (time_up != 0 && time(NULL) >= time_up) {
			break;
		}

		timer.tv_sec  = 1;
		timer.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		FD_ZERO(&xfds);

		FD_SET(fileno(stdin), &rfds);
		hfd = fileno(stdin);
		mdnssd_prepare(query, &hfd, &rfds, &wfds, &xfds);

		if ((c = select(hfd + 1, &rfds, &wfds, &xfds, &timer)) == -1) {
			util_fatal("can't select (%s)", strerror(errno));
		}
		if (c == 0) {
			util_debug("tick...");
			continue;
		}
		if (query == 1) {
			query = 0;
		}

		mdnssd_select(json, &rfds, &wfds, &xfds);

		if (FD_ISSET(0, &rfds)) {
			if (json == 0) {
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
					ofs_l = 0;
					query = 1;
					time_up = time(NULL) + timeout;
				}
			} else {
				fgets(buffer, sizeof(buffer), stdin);
				query = 1;
				time_up = time(NULL) + timeout;
			}
		}
	}

	prolog = "{\n  \"version\": 1,\n  \"result\": [\n";
	epilog = "  ]\n}\n";
	length.as_uint = strlen(prolog) + strlen(epilog);
	for (result = mdnssd_get_result(); result != NULL; result = result->next) {
		length.as_uint += strlen(result->text) + 1;
		if (result->next != NULL) {
			length.as_uint += 1;
		}
	}

	if (json == 0) {
		write(fileno(stdout), length.as_char, 4);
	} else {
		printf("%u bytes ==>\n", length.as_uint);
	}
	printf("%s", prolog);
	for (result = mdnssd_get_result(); result != NULL; result = result->next) {
		printf("%s", result->text);
		printf("%s", result->next != NULL ? ",\n" : "\n");
	}
	printf("%s", epilog);
	fflush(stdout);

	exit(EXIT_SUCCESS);
}

