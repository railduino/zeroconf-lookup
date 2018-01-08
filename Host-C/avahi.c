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


#define AVAHI_BROWSE_CMD	"/usr/bin/avahi-browse"
#define AVAHI_BROWSE_ARGS	" --resolve --parsable --no-db-lookup --terminate _http._tcp 2>/dev/null"


static char answer[1024];
static result_t *results;


int
avahi_browse(void)
{
	struct stat sb;
	FILE *fp;
	char line[4096], *ptr[32], *tmp;
	char *rr_name, *rr_target, *rr_a, *rr_port, *rr_txt;
	int num;
	result_t *result;

	if (options_use_avahi() != 1) {
		util_debug("Avahi disabled by configuration");
		return -1;
	}
	if (stat(AVAHI_BROWSE_CMD, &sb) == -1) {
		util_debug("Avahi not found where expected");
		return -1;
	}
	if ((fp = popen(AVAHI_BROWSE_CMD AVAHI_BROWSE_ARGS, "r")) == NULL) {
		util_error("Avahi found, but popen gave %s", strerror(errno));
		return -1;
	}
	util_info("using avahi-browse for discovery");

	while (fgets(line, sizeof(line), fp) != NULL) {
		for (num = 0; num < 32; num++) {
			ptr[num] = strtok(num ? NULL : line, ";\n");
			if (ptr[num] == NULL) {
				break;
			}
		}
		if (num < 9 || strcmp(ptr[0], "=") != 0) {
			continue;
		}
		if (strcmp(ptr[2], "IPv4") != 0) {
			continue;
		}
		rr_name   = ptr[3];
		rr_target = ptr[6];
		rr_a      = ptr[7];
		rr_port   = ptr[8];
		rr_txt    = (num >= 10) ? ptr[9] : "";

		for (tmp = ptr[3]; *tmp != '\0'; tmp++) {
			if (*tmp == '\\' && tmp[1] >= '0' && tmp[1] <= '9' \
					 && tmp[2] >= '0' && tmp[2] <= '9' \
					 && tmp[3] >= '0' && tmp[3] <= '9') {
				num = (int) (tmp[1] - '0') * 100 + (int) (tmp[2] - '0') * 10  + (int) (tmp[3] - '0');
				tmp[0] = (char) num;
				memmove(tmp + 1, tmp + 4, strlen(tmp + 3));
			}
		}

		if (strcmp(rr_port, "3689") == 0) {
			rr_txt = "DAAP (iTunes) Server";
		} else if (rr_txt[0] == '"') {
			rr_txt++;
			if ((num = strlen(rr_txt)) > 0 && rr_txt[num - 1] == '"') {
				rr_txt[num - 1] = '\0';
			}
		}

		UTIL_STRCPY(answer, "    {\n");
		util_append(answer, sizeof(answer), "      \"name\": \"%s\",\n",   rr_name);
		util_append(answer, sizeof(answer), "      \"txt\": \"%s\",\n",    rr_txt);
		util_append(answer, sizeof(answer), "      \"target\": \"%s\",\n", rr_target);
		util_append(answer, sizeof(answer), "      \"port\": %s,\n",       rr_port);
		util_append(answer, sizeof(answer), "      \"weight\": %u,\n",     0);
		util_append(answer, sizeof(answer), "      \"priority\": %u,\n",   0);
		util_append(answer, sizeof(answer), "      \"a\": \"%s\",\n",      rr_a);
		util_append(answer, sizeof(answer), "      \"url\": \"http://%s:%s/\"\n", rr_a, rr_port);
		UTIL_STRCAT(answer, "    }");

		result = util_malloc(sizeof(result_t));
		result->next = results;
		result->text = util_strdup(answer);
		results = result;
	}

	pclose(fp);
	return 0;
}


result_t *
avahi_get_result(void)
{
	return results;
}


static void
avahi_cleanup(void)
{
	result_t *result;

	while (results != NULL) {
		result = results->next;
		util_free(results->text);
		util_free(results);
		results = result;
	}
}


void
avahi_init(void)
{
	results = NULL;

	atexit(avahi_cleanup);
}

