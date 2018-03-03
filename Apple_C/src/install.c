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

#include <sys/types.h>
#include <sys/stat.h>


static char *dir_mozilla_system  = "/Library/Application Support/Mozilla/NativeMessagingHosts";
static char *dir_chrome_system   = "/Library/Google/Chrome/NativeMessagingHosts";
static char *dir_chromium_system = "/Library/Application Support/Chromium/NativeMessagingHosts";
static char *dir_mozilla_user    = "~/Library/Application Support/Mozilla/NativeMessagingHosts";
static char *dir_chrome_user     = "~/Library/Application Support/Google/Chrome/NativeMessagingHosts";
static char *dir_chromium_user   = "~/Library/Application Support/Chromium/NativeMessagingHosts";

static char *my_tagline          = "com.railduino.zeroconf_lookup";
static char *my_description      = "Find HTTP Servers in the .local domain using Zeroconf";
static char *my_google           = GOOGLE_TAG;
static char *my_mozilla          = MOZILLA_TAG;

static char my_executable[FILENAME_MAX];
//static char my_inst_prefix[FILENAME_MAX];


static void
install_mkdir(char *path)
{
	char buffer[FILENAME_MAX], *ptr;
	struct stat sb;

	UTIL_STRCPY(buffer, path);
	if ((ptr = strrchr(buffer, '/')) == NULL) {
		util_fatal("install_mkdir tried to create /");
	}
	*ptr = '\0';

	while (stat(buffer, &sb) == -1) {
		util_debug(__func__, __LINE__, 2, "need to mkdir %s", buffer);

		if (mkdir(buffer, 0755) == 0) {
			util_info("created directory %s", buffer);
			return;
		}

		// Try to create parent dir if needed
		if (errno == ENOENT) {
			install_mkdir(buffer);
			continue;
		}

		util_fatal("can't mkdir %s (%s)", buffer, strerror(errno));
	}

	if ((sb.st_mode & S_IFMT) != S_IFDIR) {
		util_fatal("%s is not a directory", buffer);
	}
}


static void
install_add_manifest(char *path)
{
	char filename[FILENAME_MAX];
	FILE *fp;

	if (*path == '~') {
		UTIL_STRCPY(filename, getenv("HOME"));
		UTIL_STRCAT(filename, path + 1);
	} else {
		UTIL_STRCPY(filename, path);
	}
	UTIL_STRCAT(filename, "/");
	UTIL_STRCAT(filename, my_tagline);
	UTIL_STRCAT(filename, ".json");

	install_mkdir(filename);

	if ((fp = fopen(filename, "w")) == NULL) {
		util_fatal("can't create %s (%s)", filename, strerror(errno));
	}

	fprintf(fp, "{\n");
	fprintf(fp, "  \"name\": \"%s\",\n", my_tagline);
	fprintf(fp, "  \"description\": \"%s\",\n", my_description);
	fprintf(fp, "  \"path\": \"%s\",\n", my_executable);
	fprintf(fp, "  \"type\": \"stdio\",\n");
	if (strstr(path, "zilla") != NULL) {
		fprintf(fp, "  \"allowed_extensions\": [ \"%s\" ]\n", my_mozilla);
	} else {
		fprintf(fp, "  \"allowed_origins\": [ \"chrome-extension://%s/\" ]\n", my_google);
	}
	fprintf(fp, "}\n");
	fclose(fp);

	util_info("created manifest %s", filename);
}


void
install_install(char *path)
{
	char *temp = "_install_temp_", *ptr;

	UTIL_STRCPY(my_executable, path);

	if ((ptr = strstr(my_executable, temp)) != NULL) {
		util_info("installation root is %s", temp + strlen(temp));
	}

	if (getuid() == 0) {
		install_add_manifest(dir_mozilla_system);
		install_add_manifest(dir_chrome_system);
		install_add_manifest(dir_chromium_system);
	} else {
		install_add_manifest(dir_mozilla_user);
		install_add_manifest(dir_chrome_user);
		install_add_manifest(dir_chromium_user);
	}
}


static void
install_del_manifest(char *path)
{
	char filename[FILENAME_MAX];

	if (*path == '~') {
		UTIL_STRCPY(filename, getenv("HOME"));
		UTIL_STRCAT(filename, path + 1);
	} else {
		UTIL_STRCPY(filename, path);
	}
	UTIL_STRCAT(filename, "/");
	UTIL_STRCAT(filename, my_tagline);
	UTIL_STRCAT(filename, ".json");

	if (access(filename, R_OK) == 0) {
		if (remove(filename) == -1) {
			util_fatal("can't delete %s (%s)", filename, strerror(errno));
		}
		util_info("deleted %s", filename);
	}
}


void
install_uninstall(void)
{
	if (getuid() == 0) {
		install_del_manifest(dir_mozilla_system);
		install_del_manifest(dir_chrome_system);
		install_del_manifest(dir_chromium_system);
	} else {
		install_del_manifest(dir_mozilla_user);
		install_del_manifest(dir_chrome_user);
		install_del_manifest(dir_chromium_user);
	}
}

