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

#if defined(__linux__) || defined(__APPLE__)

#if defined(__linux__)
static char *dir_mozilla_system  = "/usr/lib/mozilla/native-messaging-hosts";
static char *dir_chrome_system   = "/etc/opt/chrome/native-messaging-hosts";
static char *dir_chromium_system = "/etc/chromium/native-messaging-hosts";
static char *dir_mozilla_user    = "~/.mozilla/native-messaging-hosts";
static char *dir_chrome_user     = "~/.config/google-chrome/NativeMessagingHosts";
static char *dir_chromium_user   = "~/.config/chromium/NativeMessagingHosts";
#else
static char *dir_mozilla_system  = "/Library/Application Support/Mozilla/NativeMessagingHosts";
static char *dir_chrome_system   = "/Library/Google/Chrome/NativeMessagingHosts";
static char *dir_chromium_system = "/Library/Application Support/Chromium/NativeMessagingHosts";
static char *dir_mozilla_user    = "~/Library/Application Support/Mozilla/NativeMessagingHosts";
static char *dir_chrome_user     = "~/Library/Application Support/Google/Chrome/NativeMessagingHosts";
static char *dir_chromium_user   = "~/Library/Application Support/Chromium/NativeMessagingHosts";
#endif


static char *my_chrome  = CHROME_TAG;
static char *my_mozilla = MOZILLA_TAG;
static int   my_timeout = TIME_OUT;

static char my_executable[FILENAME_MAX];


void
install_set_chrome(char *str)
{
	my_chrome = str;
}


void
install_set_mozilla(char *str)
{
	my_mozilla = str;
}


void
install_set_timeout(char *str)
{
	int val;

	if ((val = atoi(str)) >= 1 && val <= 9) {
		my_timeout = val;
	} else {
		my_timeout = TIME_OUT;
	}
}


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
		UTIL_STRCPY(filename, path);
	}
	UTIL_STRCAT(filename, "/");
	UTIL_STRCAT(filename, HOST_NAME);
	UTIL_STRCAT(filename, ".json");

	install_mkdir(filename);

	if ((fp = fopen(filename, "w")) == NULL) {
		util_fatal("can't create %s (%s)", filename, strerror(errno));
	}

	fprintf(fp, "{\n");
	fprintf(fp, "  \"name\": \"%s\",\n", HOST_NAME);
	fprintf(fp, "  \"description\": \"%s\",\n", DESCRIPTION);
	fprintf(fp, "  \"path\": \"%s\",\n", my_executable);
	fprintf(fp, "  \"type\": \"stdio\",\n");
	if (strstr(path, "ozilla") != NULL) {
		fprintf(fp, "  \"allowed_extensions\": [ \"%s\" ]\n", MOZILLA_TAG);
	} else {
		fprintf(fp, "  \"allowed_origins\": [ \"chrome-extension://%s/\" ]\n", CHROME_TAG);
	}
	fprintf(fp, "}\n");

	fclose(fp);
	util_info("created %s", filename);
}


void
install_install(char *prog)
{
	char *ptr;
#if defined(__linux__)
	char env_path[FILENAME_MAX];
	struct stat sb;

	if (prog == NULL) {
		util_fatal("can't determine my own executable");
	}
	if (*prog == '/') {
		UTIL_STRCPY(my_executable, prog);
	} else if (strchr(prog, '/') != NULL) {
		getcwd(my_executable, sizeof(my_executable));
		UTIL_STRCAT(my_executable, "/");
		UTIL_STRCAT(my_executable, prog);
	} else {
		UTIL_STRCPY(env_path, getenv("PATH"));
		for (ptr = strtok(env_path, ":"); ptr != NULL; ptr = strtok(NULL, ":")) {
			UTIL_STRCPY(my_executable, ptr);
			UTIL_STRCAT(my_executable, "/");
			UTIL_STRCAT(my_executable, prog);
			if (stat(my_executable, &sb) == 0) {
				break;
			}
		}
		if (ptr == NULL) {
			util_fatal("can't locate my own executable");
		}
	}
	if (access(my_executable, X_OK) == -1) {
		util_fatal("can't access my own executable");
	}
#else
	uint32_t size = sizeof(my_executable);
	(void) prog;    // not necessary

	if (_NSGetExecutablePath(my_executable, &size) != 0) {
		util_fatal("can't access my own executable");
	}
#endif

	while ((ptr = strstr(my_executable, "/./")) != NULL) {
		memmove(ptr, ptr + 2, strlen(ptr));
	}

	util_info("executable is located at %s", my_executable);

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
	UTIL_STRCAT(filename, HOST_NAME);
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
#if defined(__linux__) || defined(__APPLE__)
	if (getuid() == 0) {
		install_del_manifest(dir_mozilla_system);
		install_del_manifest(dir_chrome_system);
		install_del_manifest(dir_chromium_system);
	} else {
		install_del_manifest(dir_mozilla_user);
		install_del_manifest(dir_chrome_user);
		install_del_manifest(dir_chromium_user);
	}
#else
	util_fatal("sorry -- not yet implemented");
#endif
}

#endif /* defined(__linux__) || defined(__APPLE__) */

