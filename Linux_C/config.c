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


static char my_google[256];
static char my_mozilla[256];
static char my_timeout[32];
static char my_force[32];

static char *my_cfgfile = CONFIG_FILE;


static void
config_set_google(char *val, char *auth)
{
	if (val == NULL) {
		util_fatal("missing google [%s]", auth);
	}
	if (strlen(val) != 32) {
		util_fatal("invalid google '%s' [%s]", val, auth);
	}

	UTIL_STRCPY(my_google, val);
	util_info("[%s] google  '%s'", auth, my_google);
}


char *
config_get_google(void)
{
	return my_google;
}


static void
config_set_mozilla(char *val, char *auth)
{
	if (val == NULL) {
		util_fatal("missing mozilla [%s]", auth);
	}
	if (strchr(val, '@') == NULL) {
		util_fatal("invalid mozilla '%s' [%s]", val, auth);
	}

	UTIL_STRCPY(my_mozilla, val);
	util_info("[%s] mozilla '%s'", auth, my_mozilla);
}


char *
config_get_mozilla(void)
{
	return my_mozilla;
}


static void
config_set_timeout(char *val, char *auth)
{
	int num;

	if (val == NULL) {
		util_fatal("missing timeout [%s]", auth);
	}
	if ((num = atoi(val)) < 1 || num > 59) {
		util_fatal("invalid timeout %d [%s] (only 1 to 59)", num, auth);
	}

	snprintf(my_timeout, sizeof(my_timeout), "%d", num);
	util_info("[%s] timeout '%s'", auth, my_timeout);
}


int
config_get_timeout(void)
{
	return atoi(my_timeout);
}


static void
config_set_force(char *val, char *auth)
{
	if (val == NULL) {
		util_fatal("missing force [%s]", auth);
	}
	if (*val && strcmp(val, "avahi") != 0 && strcmp(val, "query") != 0) {
		util_fatal("invalid force '%s' [%s] (only avahi or query)", val, auth);
	}
	UTIL_STRCPY(my_force, val);
	util_info("[%s] force   '%s'", auth, my_force);
}


char *
config_get_force(void)
{
	return my_force;
}


void
config_read(char *google, char *mozilla, char *timeout, char *force)
{
	static char *inst = "install", *conf = "cfgfile", *argv = "cmdline";
	FILE *fp;
	char line[1024], *var, *val;

	config_set_google(GOOGLE_TAG,   inst);
	config_set_mozilla(MOZILLA_TAG, inst);
	config_set_timeout(TIME_OUT,    inst);
	config_set_force(FORCE_METHOD,  inst);

	if ((fp = fopen(my_cfgfile, "r")) != NULL) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			if ((var = strtok(line, "=\n")) == NULL) {
				continue;
			}
			if (*var == '\0' || *var == '#') {
				continue;
			}
			if ((val = strtok(NULL, "\n")) == NULL) {
				continue;
			}
			if (strcmp(var, "google") == 0) {
				config_set_google(val, conf);
				continue;
			}
			if (strcmp(var, "mozilla") == 0) {
				config_set_mozilla(val, conf);
				continue;
			}
			if (strcmp(var, "timeout") == 0) {
				config_set_timeout(val, conf);
				continue;
			}
			if (strcmp(var, "force") == 0) {
				config_set_force(val, conf);
				continue;
			}
			util_info("ignore config line '%s=%s'", var, val);
		}
		fclose(fp);
	}

	if (*google != '\0') {
		config_set_google(google, argv);
	}
	if (*mozilla != '\0') {
		config_set_mozilla(mozilla, argv);
	}
	if (*timeout != '\0') {
		config_set_timeout(timeout, argv);
	}
	if (*force != '\0') {
		config_set_force(force, argv);
	}
}

