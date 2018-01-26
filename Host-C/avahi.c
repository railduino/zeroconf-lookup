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

#if defined(HAVE_AVAHI)

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>


static result_t *my_results = NULL;

static AvahiSimplePoll     *my_poll    = NULL;
static AvahiClient         *my_client  = NULL;
static AvahiServiceBrowser *my_browser = NULL;


static void
avahi_cleanup(void)
{
	result_t *tmp;

	if (my_browser != NULL) {
		avahi_service_browser_free(my_browser);
		my_browser = NULL;
	}

	if (my_client != NULL) {
		avahi_client_free(my_client);
		my_client = NULL;
	}

	if (my_poll != NULL) {
		avahi_simple_poll_free(my_poll);
		my_poll = NULL;
	}

	while (my_results != NULL) {
		tmp = my_results->next;
		util_free(my_results->text);
		util_free(my_results);
		my_results = tmp;
	}
}


static void
avahi_resolve_callback(AvahiServiceResolver *r,
		AVAHI_GCC_UNUSED AvahiIfIndex interface,
		AvahiProtocol protocol,
		AvahiResolverEvent event,
		const char *name,
		AVAHI_GCC_UNUSED const char *type,
		AVAHI_GCC_UNUSED const char *domain,
		const char *host_name,
		const AvahiAddress *address,
		uint16_t port,
		AvahiStringList *txt,
		AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
		AVAHI_GCC_UNUSED void *userdata)
{
	char answer[4096], tmp_adr[AVAHI_ADDRESS_STR_MAX], url[1024];
	txt_t *head, *tail, *ptr;
	AvahiStringList *run;
	result_t *result;
	size_t len;

	if (event == AVAHI_RESOLVER_FAILURE) {
		util_error("avahi_resolve_callback() error %s", avahi_strerror(avahi_client_errno(avahi_service_resolver_get_client(r))));
		avahi_service_resolver_free(r);
		return;
	}
	if (event != AVAHI_RESOLVER_FOUND) {
		util_debug(3, "avahi_resolve_callback() unknown event %d", event);
		avahi_service_resolver_free(r);
		return;
	}
	util_debug(3, "avahi_resolve_callback() event: AVAHI_RESOLVER_FOUND %s", host_name);

	if (protocol == AVAHI_PROTO_INET6) {
		util_debug(3, "avahi_resolve_callback() ignore IPv6");
		avahi_service_resolver_free(r);
		return;
	}

	avahi_address_snprint(tmp_adr, sizeof(tmp_adr), address);
	snprintf(url, sizeof(url), "http://%s:%u/", tmp_adr, port);

	for (run = txt, tail = NULL; run != NULL; run = run->next) {
		ptr = util_malloc(sizeof(txt_t) + run->size);
		util_strcpy(ptr->text, (char *) run->text, run->size + 1);
		if (tail == NULL) {
			tail = (head = ptr);
		} else {
			tail = (tail->next = ptr);
		}
	}
	if (port == 3689) {
		char *str = "DAAP (iTunes) Server";
		len = strlen(str);
		ptr = util_malloc(sizeof(txt_t) + len);
		util_strcpy(ptr->text, str, len + 1);
		ptr->next = head;
		head = ptr;
	}

	UTIL_STRCPY(answer, "    {\n");
	util_append(answer, sizeof(answer), "      \"name\": \"%s\",\n",   name);

	util_append(answer, sizeof(answer), "      \"txt\": [ ");
	for (ptr = head; ptr != NULL; ptr = ptr->next) {
		util_append(answer, sizeof(answer), "\"%s\"", ptr->text);
		if (ptr->next != NULL) {
			util_append(answer, sizeof(answer), ", ");
		} else {
			util_append(answer, sizeof(answer), " ");
		}
	}
	util_append(answer, sizeof(answer), "],\n");

	util_append(answer, sizeof(answer), "      \"target\": \"%s\",\n", host_name);
	util_append(answer, sizeof(answer), "      \"port\": %u,\n",       port);
	util_append(answer, sizeof(answer), "      \"a\": \"%s\",\n",      tmp_adr);
	util_append(answer, sizeof(answer), "      \"url\": \"%s\"\n",     url);
	UTIL_STRCAT(answer, "    }");

	while (head != NULL) {
		ptr = head->next;
		util_free(head);
		head = ptr;
	}

	avahi_service_resolver_free(r);

	for (result = my_results; result != NULL; result = result->next) {
		if (strcmp(result->text, answer) == 0) {
			util_debug(1, "Avahi duplicate: %s", name);
			return;		// duplicate entry
		}
	}

	result = util_malloc(sizeof(result_t));
	result->next = my_results;
	result->text = util_strdup(answer);
	my_results = result;

	util_info("Avahi found %s for %s", url, name);
}


static void
avahi_browse_callback(AvahiServiceBrowser *b,
		AvahiIfIndex interface,
		AvahiProtocol protocol,
		AvahiBrowserEvent event,
		const char *name,
		const char *type,
		const char *domain,
		AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
		void *userdata)
{
	AvahiClient *c = userdata;

	if (event == AVAHI_BROWSER_FAILURE) {
		util_error("avahi_browse_callback() error %s", avahi_strerror(avahi_client_errno(avahi_service_browser_get_client(b))));
		avahi_simple_poll_quit(my_poll);
		return;
	}

	if (event == AVAHI_BROWSER_NEW) {
		if (avahi_service_resolver_new(c, interface, protocol, name, type, domain,
				AVAHI_PROTO_UNSPEC, 0, avahi_resolve_callback, c) == NULL) {
			util_error("avahi_browse_callback() error for %s: %s", name, avahi_strerror(avahi_client_errno(c)));
		}
		util_debug(3, "avahi_browse_callback() event: AVAHI_BROWSER_NEW");
		return;
	}

	if (event == AVAHI_BROWSER_ALL_FOR_NOW) {
		util_debug(3, "avahi_browse_callback() event: AVAHI_BROWSER_ALL_FOR_NOW");
		avahi_simple_poll_quit(my_poll);
	} else {
		util_debug(3, "avahi_browse_callback() event: %d", (int) event);
	}
}


static void
avahi_client_callback(AvahiClient *c,
		AvahiClientState state,
		AVAHI_GCC_UNUSED void *userdata)
{
	if (state == AVAHI_CLIENT_FAILURE) {
		util_error("avahi_client_callback() error %s", avahi_strerror(avahi_client_errno(c)));
		avahi_simple_poll_quit(my_poll);
	}

	util_debug(3, "avahi_client_callback() state: %d", (int) state);
}


result_t *
avahi_browse(void)
{
	int error;

	util_info("calling Avahi browser");
	atexit(avahi_cleanup);

	my_poll = avahi_simple_poll_new();
	if (my_poll == NULL) {
		util_error("avahi_simple_poll_new() failed");
		return NULL;
	}
	util_debug(3, "success: avahi_simple_poll_new()");

	my_client = avahi_client_new(avahi_simple_poll_get(my_poll), 0, avahi_client_callback, NULL, &error);
	if (my_client == NULL) {
		util_error("avahi_client_new() error %s", avahi_strerror(error));
		return NULL;
	}
	util_debug(3, "success: avahi_client_new()");

	my_browser = avahi_service_browser_new(my_client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
			"_http._tcp", NULL, 0, avahi_browse_callback, my_client);
	if (my_browser == NULL) {
		fprintf(stderr, "avahi_service_browser_new() error %s", avahi_strerror(avahi_client_errno(my_client)));
		return NULL;
	}
	util_debug(3, "success: avahi_service_browser_new()");

	avahi_simple_poll_loop(my_poll);

	return my_results;
}

#endif /* defined(HAVE_AVAHI) */

