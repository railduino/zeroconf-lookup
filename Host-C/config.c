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


#define DEFAULT_TIMEOUT		3
#define DEFAULT_AVAHI		1
#define DEFAULT_DEBUG		0


static int timeout = DEFAULT_TIMEOUT;
static int avahi   = DEFAULT_AVAHI;
static int debug   = DEFAULT_DEBUG;

int
config_get_timeout(void)
{
	return timeout;
}


int
config_use_avahi(void)
{
	return avahi;
}


int
config_use_debug(void)
{
	return debug;
}


static void
config_read_value(char *line)
{
	int result;

	if (sscanf(line, "timeout=%d", &result) == 1) {
		if (result >= 1 && result <= 9) {
			timeout = result;
		} else {
			util_error("timeout must be from 1 to 9, but is %d", result);
		}
		return;
	}

	if (sscanf(line, "avahi=%d", &result) == 1) {
		if (result == 0 || result == 1) {
			avahi = result;
		} else {
			util_error("avahi must be 0 or 1, but is %d", result);
		}
		return;
	}

	if (sscanf(line, "debug=%d", &result) == 1) {
		if (result == 0 || result == 1) {
			debug = result;
		} else {
			util_error("debug must be 0 or 1, but is %d", result);
		}
		return;
	}
}


void
config_init(char *argv0)
{
	char config[FILENAME_MAX], line[1024], *ptr;
	FILE *fp = NULL;

	if ((ptr = getenv("ZEROCONF_LOOKUP")) != NULL) {
		UTIL_STRCPY(config, ptr);
		for (ptr = strtok(config, ",:;"); ptr != NULL; ptr = strtok(NULL, ",:;")) {
			config_read_value(ptr);
		}
	}

	if (fp == NULL && (ptr = getenv("HOME")) != NULL) {
		UTIL_STRCPY(config, ptr);
		UTIL_STRCAT(config, "/zeroconf_lookup.conf");
		ptr = config;
		fp = fopen(ptr, "r");
	}
	if (fp == NULL) {
		ptr = "/etc/zeroconf_lookup.conf";
		fp = fopen(ptr, "r");
	}
	if (fp == NULL) {
		UTIL_STRCPY(config, argv0);
		UTIL_STRCAT(config, ".conf");
		ptr = config;
		fp = fopen(ptr, "r");
	}
	if (fp == NULL) {
		// no config, will use defaults
		return;
	}
	util_info("read config from %s", ptr);

	while (fgets(line, sizeof(line), fp) != NULL) {
		config_read_value(line);
	}

	fclose(fp);
}

