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

#if defined(HAVE_DNSSD)

#include <dns_sd.h>
#include <arpa/inet.h>

#define POLL_TIMEOUT	1000


typedef struct _record {
	struct _record *next;
	DNSServiceRef  *newref;
	char	*replyName;
	char	*replyType;
	char	*replyDomain;
	char	*hostname;
	char	*address;
	int	port;
	txt_t	*txt;
} record_t;


static result_t *my_results = NULL;

static DNSServiceRef  my_client  = NULL;
static int            my_done;
static record_t      *my_records = NULL;


static void
dnssd_cleanup(void)
{
	if (my_client != NULL) {
		DNSServiceRefDeallocate(my_client);
		my_client = NULL;
	}

	while (my_records != NULL) {
		record_t *tmp = my_records->next;

		util_free(my_records->replyName);
		util_free(my_records->replyType);
		util_free(my_records->replyDomain);
		util_free(my_records->hostname);
		util_free(my_records->address);

		while (my_records->txt != NULL) {
			txt_t *txt = my_records->txt->next;
			util_free(my_records->txt);
			my_records->txt = txt;
		}

		util_free(my_records);
		my_records = tmp;
	}

	while (my_results != NULL) {
		result_t *tmp = my_results->next;
		util_free(my_results->text);
		util_free(my_results);
		my_results = tmp;
	}
}


static void DNSSD_API
dnssd_zonedata_resolve(DNSServiceRef  sdref,
		const DNSServiceFlags flags,
		uint32_t              ifIndex,
		DNSServiceErrorType   errorCode,
		const char            *fullname,
		const char            *hosttarget,
		uint16_t              opaqueport,
		uint16_t              txtLen,
		const unsigned char   *txt,
		void                  *context)
{
	int port = htons(opaqueport);
	record_t *record = (record_t *) context;
	size_t ofs, len;
	txt_t *tail, *ptr;

	util_debug(2, "callback dnssd_zonedata_resolve()");
	util_debug(3, "dnssd_zonedata_resolve sdref=0x%x flags=0x%x ifIndex=%d fullname=0x%x",
			(unsigned) sdref, (unsigned) flags, ifIndex, (unsigned) fullname);

	if (errorCode != kDNSServiceErr_NoError) {
		util_fatal("dnssd_zonedata_resolve errorCode %d", errorCode);
	}

	record->hostname = util_strdup(hosttarget);
	record->port     = port;
	util_info("'%s' -> %s:%d", record->replyName, record->hostname, record->port);

	if (txt != NULL && txtLen > 0 && txtLen < 256) {
		for (ofs = 0, tail = NULL; ofs < txtLen && txt[ofs] != 0; ) {
			len = (size_t) txt[ofs];
			ptr = util_malloc(sizeof(txt_t) + len);
			util_strcpy(ptr->text, (const char *) txt + ofs + 1, len + 1);
			ofs += (1 + len);
			if (tail == NULL) {
				tail = (record->txt = ptr);
			} else {
				tail = (tail->next = ptr);
			}
			util_info("    TXT: '%s'", ptr->text);
		}
	}

	DNSServiceRefDeallocate(sdref);
	util_free(record->newref);
	record->newref = NULL;

	if ((flags & kDNSServiceFlagsMoreComing) == 0) {
		my_done = 1;
	}
}


static void DNSSD_API
dnssd_zonedata_browse(DNSServiceRef   sdref,
		const DNSServiceFlags flags,
		uint32_t              ifIndex,
		DNSServiceErrorType   errorCode,
		const char            *replyName,
		const char            *replyType,
		const char            *replyDomain,
		void                  *context)
{
	record_t *record;

	if ((flags & kDNSServiceFlagsAdd) == 0) {
		return;
	}

	util_debug(2, "callback dnssd_zonedata_browse() if=%d %s / %s / %s",
			ifIndex, replyName, replyType, replyDomain);
	util_debug(3, "dnssd_zonedata_browse sdref=0x%x flags=0x%x context=0x%x",
			(unsigned) sdref, (unsigned) flags, (unsigned) context);

	if (errorCode != kDNSServiceErr_NoError) {
		util_fatal("dnssd_zonedata_browse errorCode %d", errorCode);
	}

	record = util_malloc(sizeof(record_t));
	record->replyName   = util_strdup(replyName);
	record->replyType   = util_strdup(replyType);
	record->replyDomain = util_strdup(replyDomain);
	record->next        = my_records;
	my_records = record;

	util_info("found '%s'", replyName);

	if ((flags & kDNSServiceFlagsMoreComing) == 0) {
		my_done = 1;
	}
}


static void DNSSD_API
dnssd_addrinfo_reply(DNSServiceRef    sdref,
		const DNSServiceFlags flags,
		uint32_t              interfaceIndex,
		DNSServiceErrorType   errorCode,
		const char            *hostname,
		const struct sockaddr *address,
		uint32_t              ttl,
		void                  *context)
{
	char buffer[INET_ADDRSTRLEN];
	record_t *record = (record_t *) context;

	util_debug(2, "callback dnssd_addrinfo_reply() %s", hostname);
	util_debug(3, "dnssd_addrinfo_reply sdref=0x%x flags=0x%x interfaceIndex=%d ttl=%d",
			(unsigned) sdref, (unsigned) flags, interfaceIndex, ttl);

	if (errorCode != kDNSServiceErr_NoError) {
		util_fatal("dnssd_addrinfo_reply errorCode %d", errorCode);
	}

	if (address && address->sa_family == AF_INET) {
		struct sockaddr_in sin;
		memcpy(&sin, (const char *) address, sizeof(sin));
		if (inet_ntop(AF_INET, &(sin.sin_addr), buffer, INET_ADDRSTRLEN) == NULL) {
			util_fatal("dnssd_addrinfo_reply error %s", strerror(errno));
		}
		record->address = util_strdup(buffer);
		util_info("%s (%s) -> %s", hostname, record->replyName, record->address);
	}

	my_done = 1;
}


static void
dnssd_event_loop(char *target)
{
	struct pollfd fds[1];
	int err;

	util_debug(2, "dnssd_event_loop %s", target);

	for (my_done = 0; my_done == 0; ) {
		fds[0].fd = DNSServiceRefSockFD(my_client);
		fds[0].events = POLLIN;

		if ((err = poll(fds, 1, POLL_TIMEOUT)) == -1) {
			util_fatal("can't poll target (%s)", target, strerror(errno));
		}
		if (err == 0) {
			util_debug(1, "timeout %s", target);
			my_done = 1;
		}

		if (fds[0].revents & POLLIN) {
			err = DNSServiceProcessResult(my_client);
			if (err != kDNSServiceErr_NoError) {
				util_fatal("DNSServiceProcessResult %s error %d", target, err);
			}
		}
	}
}


result_t *
dnssd_browse(void)
{
	int err;
	DNSServiceRef browser;
	record_t *record;
	char answer[4096], url[1024];
	result_t *result;
	txt_t *txt;

	atexit(dnssd_cleanup);

	util_debug(1, "call DNSServiceCreateConnection()");
	err = DNSServiceCreateConnection(&my_client);
	if (err != kDNSServiceErr_NoError) {
		util_fatal("DNSServiceCreateConnection error %d", err);
	}
	util_debug(2, "DNSServiceCreateConnection fd=%d", DNSServiceRefSockFD(my_client));

	//
	// Step 1: collect all servers
	//
	browser = my_client;
	err = DNSServiceBrowse(&browser,
			kDNSServiceFlagsShareConnection,
			kDNSServiceInterfaceIndexAny,
			"_http._tcp",
			"local",
			dnssd_zonedata_browse,
			NULL);
	if (err != kDNSServiceErr_NoError) {
		util_fatal("DNSServiceBrowse error %d", err);
	}
	dnssd_event_loop("collect");

	//
	// Step 2: Get hostname and port
	//
	for (record = my_records; record != NULL; record = record->next) {
		record->newref = util_malloc(sizeof(DNSServiceRef));
		*(record->newref) = my_client;
		util_debug(2, "get server/port for %s", record->replyName);
		DNSServiceResolve(record->newref,
				kDNSServiceFlagsShareConnection,
				kDNSServiceInterfaceIndexAny,
				record->replyName,
				record->replyType,
				record->replyDomain,
				dnssd_zonedata_resolve,
				(void *) record);
		dnssd_event_loop(record->replyName);
	}

	//
	// The browser client is not needed any more
	//
	DNSServiceRefDeallocate(my_client);
	my_client = NULL;

	//
	// Step 3: Get IP address
	//
	for (record = my_records; record != NULL; record = record->next) {
		util_debug(2, "get IP address for %s", record->hostname);
		err = DNSServiceGetAddrInfo(&my_client,
				kDNSServiceFlagsReturnIntermediates,
				kDNSServiceInterfaceIndexAny,
				kDNSServiceProtocol_IPv4,
				record->hostname,
				dnssd_addrinfo_reply,
				(void *) record);
		if (err != kDNSServiceErr_NoError) {
			util_fatal("DNSServiceGetAddrInfo %s error %d", record->hostname, err);
		}

		dnssd_event_loop(record->hostname);

		DNSServiceRefDeallocate(my_client);
		my_client = NULL;
	}

	//
	// Step 4: Assemble JSON response
	//
	for (record = my_records; record != NULL; record = record->next) {
		if (record->port == 3689) {
			txt = util_malloc(sizeof(txt_t));
			txt->text = util_strdup("DAAP (iTunes) Server");
			txt->next = record->txt;
			record->txt = txt;
		}
		snprintf(url, sizeof(url), "http://%s:%u/", record->address, record->port);
		(void) util_strtrim(record->hostname, ".");

		UTIL_STRCPY(answer, "    {\n");
		util_append(answer, sizeof(answer), "      \"name\": \"%s\",\n",   record->replyName);

		util_append(answer, sizeof(answer), "      \"txt\": [ ");
		for (txt = record->txt; txt != NULL; txt = txt->next) {
			util_append(answer, sizeof(answer), "\"%s\"", txt->text);
			if (txt->next != NULL) {
				util_append(answer, sizeof(answer), ", ");
			} else {
				util_append(answer, sizeof(answer), " ");
			}
		}
		util_append(answer, sizeof(answer), "],\n");

		util_append(answer, sizeof(answer), "      \"target\": \"%s\",\n", record->hostname);
		util_append(answer, sizeof(answer), "      \"port\": %u,\n",       record->port);
		util_append(answer, sizeof(answer), "      \"a\": \"%s\",\n",      record->address);
		util_append(answer, sizeof(answer), "      \"url\": \"%s\"\n",     url);
		UTIL_STRCAT(answer, "    }");

		for (result = my_results; result != NULL; result = result->next) {
			if (strcmp(result->text, answer) == 0) {
				util_debug(1, "mDNSResponder duplicate: %s", record->replyName);
				break;		// duplicate entry
			}
		}
		if (result == NULL) {
			result = util_malloc(sizeof(result_t));
			result->next = my_results;
			result->text = util_strdup(answer);
			my_results = result;
		}
	}

	return my_results;
}

#endif /* defined(HAVE_DNSSD) */

