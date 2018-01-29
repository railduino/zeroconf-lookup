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


typedef struct _option {
	struct _option	*next;
	char		*name;
	char		*text;
} option_t;


static option_t *my_options = NULL;


char *
options_get_string(char *name, char *dflt)
{
	option_t *option;

	util_debug(1, "check option %s (%s)", name, dflt);

	for (option = my_options; option != NULL; option = option->next) {
		if (strcasecmp(name, option->name) == 0) {
			util_debug(1, "(found): '%s'", option->text);
			return option->text;
		}
	}

	util_debug(1, "(default): '%s'", dflt);
	return dflt;
}


int
options_get_number(char *name, int dflt, int min, int max)
{
	char temp[32], *ptr;
	int val;

	snprintf(temp, sizeof(temp), "%d", dflt);
	ptr = options_get_string(name, temp);

	if ((val = atoi(ptr)) < min) {
		util_debug(1, "(using min): %d", min);
		return min;
	}
	if (val > max) {
		util_debug(1, "(using max): %d", max);
		return max;
	}

	util_debug(1, "(using val): %d", val);
	return val;
}


static void
options_scan_line(char *line)
{
	option_t *option;
	char *ptr;

	if (line == NULL || *line == '#' || *line == '\n') {
		return;
	}

	if ((ptr = strchr(line, '=')) != NULL) {
		*ptr++ = '\0';
		option = util_malloc(sizeof(option_t));
		option->name = util_strdup(util_strtrim(line, NULL));
		option->text = util_strdup(util_strtrim(ptr,  NULL));
		option->next = my_options;
		my_options = option;
	}
}


static void
options_cleanup(void)
{
	option_t *option;

	while (my_options != NULL) {
		option = my_options->next;
		util_free(my_options->name);
		util_free(my_options->text);
		util_free(my_options);
		my_options = option;
	}
}


void
options_init(void)
{
	char config[FILENAME_MAX], line[1024], *ptr;
	FILE *fp = NULL;

	atexit(options_cleanup);

	if ((ptr = getenv("ZEROCONF_LOOKUP")) != NULL) {
		UTIL_STRCPY(config, ptr);
		for (ptr = strtok(config, ",\n"); ptr != NULL; ptr = strtok(NULL, ",\n")) {
			options_scan_line(ptr);
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
		util_info("use default config");
		// no config, will use empty strings
		return;
	}
	util_info("read config from %s", ptr);

	while (fgets(line, sizeof(line), fp) != NULL) {
		options_scan_line(line);
	}

	fclose(fp);
}

