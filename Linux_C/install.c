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

#include <sys/types.h>
#include <sys/stat.h>


static char *dir_mozilla_system  = "/usr/lib/mozilla/native-messaging-hosts";
static char *dir_chrome_system   = "/etc/opt/chrome/native-messaging-hosts";
static char *dir_chromium_system = "/etc/chromium/native-messaging-hosts";
static char *dir_mozilla_user    = "~/.mozilla/native-messaging-hosts";
static char *dir_chrome_user     = "~/.config/google-chrome/NativeMessagingHosts";
static char *dir_chromium_user   = "~/.config/chromium/NativeMessagingHosts";

static char *my_tagline          = "com.railduino.zeroconf_lookup";
static char *my_description      = "Find HTTP Servers in the .local domain using Zeroconf";

static char *my_destdir = NULL;
static char  my_virt_exec[FILENAME_MAX];
static char  my_real_exec[FILENAME_MAX];


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
		util_debug(2, "need to mkdir %s", buffer);

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
		if (my_destdir != NULL) {
			UTIL_STRCPY(filename, my_destdir);
			UTIL_STRCAT(filename, path);
		} else {
			UTIL_STRCPY(filename, path);
		}
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
	fprintf(fp, "  \"path\": \"%s\",\n", my_virt_exec);
	fprintf(fp, "  \"type\": \"stdio\",\n");
	if (strstr(path, "zilla") != NULL) {
		fprintf(fp, "  \"allowed_extensions\": [ \"%s\" ]\n", config_get_mozilla());
	} else {
		fprintf(fp, "  \"allowed_origins\": [ \"chrome-extension://%s/\" ]\n", config_get_google());
	}
	fprintf(fp, "}\n");
	fclose(fp);

	util_info("created manifest %s", filename);
}


void
install_install(char *prog)
{
	char filename[FILENAME_MAX], *ptr;
	struct stat sb;
	FILE *fp;

	if (prog == NULL) {
		util_fatal("can't determine my own executable");
	}

	if ((my_destdir = getenv("DESTDIR")) != NULL) {
		if (getuid() != 0 || *prog != '/') {
			util_fatal("using DESTDIR requires root install");
		}
	}

	if (*prog == '/') {
		UTIL_STRCPY(my_real_exec, prog);
		if (my_destdir != NULL) {
			while ((ptr = strstr(my_real_exec, "//")) != NULL) {
				memmove(ptr, ptr + 1, strlen(ptr));
			}
			ptr = my_real_exec + strlen(my_destdir);
			UTIL_STRCPY(my_virt_exec, ptr);
		} else {
			UTIL_STRCPY(my_virt_exec, my_real_exec);
		}
	} else if (strchr(prog, '/') != NULL) {
		getcwd(my_real_exec, sizeof(my_real_exec));
		UTIL_STRCAT(my_real_exec, "/");
		UTIL_STRCAT(my_real_exec, prog);
		while ((ptr = strstr(my_real_exec, "/./")) != NULL) {
			memmove(ptr, ptr + 2, strlen(ptr));
		}
		UTIL_STRCPY(my_virt_exec, my_real_exec);
	} else {
		UTIL_STRCPY(filename, getenv("PATH"));
		for (ptr = strtok(filename, ":"); ptr != NULL; ptr = strtok(NULL, ":")) {
			UTIL_STRCPY(my_real_exec, ptr);
			UTIL_STRCAT(my_real_exec, "/");
			UTIL_STRCAT(my_real_exec, prog);
			if (stat(my_real_exec, &sb) == 0) {
				break;
			}
		}
		if (ptr == NULL) {
			util_fatal("can't locate my own executable");
		}
		UTIL_STRCPY(my_virt_exec, my_real_exec);
	}

	if (access(my_real_exec, X_OK) == -1) {
		util_fatal("can't access my own executable");
	}
	util_info("executable is located at %s", my_real_exec);

	if (getuid() == 0) {
		install_add_manifest(dir_mozilla_system);
		install_add_manifest(dir_chrome_system);
		install_add_manifest(dir_chromium_system);
	} else {
		install_add_manifest(dir_mozilla_user);
		install_add_manifest(dir_chrome_user);
		install_add_manifest(dir_chromium_user);
	}

	if (my_destdir != NULL) {
		UTIL_STRCPY(filename, my_destdir);
		UTIL_STRCAT(filename, CONFIG_FILE);
	} else {
		UTIL_STRCPY(filename, CONFIG_FILE);
	}
	if ((fp = fopen(filename, "w")) == NULL) {
		util_fatal("can't create %s (%s)", filename, strerror(errno));
	}
	fprintf(fp, "#\n");
	fprintf(fp, "# Configuration for zeroconf_lookup\n");
	fprintf(fp, "# See: https://www.railduino.de/zeroconf-lookup\n");
	fprintf(fp, "#\n");
	fprintf(fp, "#google=%s\n",  config_get_google());
	fprintf(fp, "#mozilla=%s\n", config_get_mozilla());
	fprintf(fp, "#timeout=%d\n", config_get_timeout());
	fprintf(fp, "#force=%s\n",   config_get_force());
	fprintf(fp, "\n");
	fclose(fp);
	util_info("created cfg-file %s", filename);
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

