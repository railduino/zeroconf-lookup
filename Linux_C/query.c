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
#include "parser.h"

#include <poll.h>


#define MDNS_SIZE	4096
#define QUERY_NAME	"_http._tcp.local"

#define INADDR_MDNS	"224.0.0.251"
#define MDNS_PORT	5353


static result_t *my_results = NULL;
static int       my_sock    = 0;


static void
query_cleanup(void)
{
	struct ip_mreq mreq;
	result_t *result;

	if (my_sock > 0) {
		mreq.imr_multiaddr.s_addr = inet_addr(INADDR_MDNS);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		setsockopt(my_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));

		close(my_sock);
		my_sock = 0;
	}

	while (my_results != NULL) {
		result = my_results->next;
		util_free(my_results->text);
		util_free(my_results);
		my_results = result;
	}
}


static void
query_read_answer(void)
{
	char buf[MDNS_SIZE], url[MDNS_SIZE], answer[MDNS_SIZE];
	struct sockaddr_storage addr;
	socklen_t len;
	int cnt, res, num, port;
	DNS_RR rrs[10], *rrp;
	DNS_RR_TXT *txt;
	char *ipv4, *ipv6, *name, *ptr;
	result_t *result;

	len = sizeof(addr);
	cnt = recvfrom(my_sock, buf, sizeof(buf), 0, (struct sockaddr *) &addr, &len);
	buf[cnt] = '\0';

	if ((res = parser_parse_answer(buf, cnt, rrs, sizeof(rrs))) == -1) {
		util_error(__func__, __LINE__, "%s", parser_get_error());
		return;
	}
	if (res == 0) {
		util_debug(3, "query: got DNS message, but no answer");
		return;
	}

	UTIL_STRCPY(answer, "    {\n");
	port = 0;
	ipv4 = ipv6 = name = NULL;
	txt = NULL;

	for (num = 0, rrp = rrs; num < res; num++, rrp++) {
		if (rrp->rr_type == DNS_RR_TYPE_A) {
			util_append(answer, sizeof(answer), "      \"a\": \"%s\",\n", rrp->rr.rr_a.a_addr_str);
			if (ipv4 == NULL) {
				ipv4 = rrp->rr.rr_a.a_addr_str;
			}
			continue;
		}

		if (rrp->rr_type == DNS_RR_TYPE_PTR) {
			name = rrp->rr.rr_ptr.ptr_dname;
			if ((ptr = strstr(name, "._http._tcp.local")) != NULL) {
				*ptr = '\0';
			}
			util_append(answer, sizeof(answer), "      \"name\": \"%s\",\n", name);
			continue;
		}

		if (rrp->rr_type == DNS_RR_TYPE_TXT) {
			txt = &(rrp->rr.rr_txt);		// will check for iTunes
			continue;
		}

		//
		// Ignore IPv6 address for now
		//
		//if (rrp->rr_type == DNS_RR_TYPE_AAAA) {
		//	util_append(answer, sizeof(answer), "      \"aaaa\": \"%s\",\n", rrp->rr.rr_aaaa.aaaa_addr_str);
		//	if (ipv6 == NULL) {
		//		ipv6 = rrp->rr.rr_a.a_addr_str;
		//	}
		//	continue;
		//}

		if (rrp->rr_type == DNS_RR_TYPE_SRV) {
			port = rrp->rr.rr_srv.srv_port;
			util_append(answer, sizeof(answer), "      \"target\": \"%s\",\n", rrp->rr.rr_srv.srv_target);
			util_append(answer, sizeof(answer), "      \"port\": %u,\n",       port);
			continue;
		}
	}

	if (name == NULL) {
		util_debug(1, "query: incomplete answer (missing name)");
		return;
	}
	if (ipv4 == NULL) {
		util_debug(1, "query: incomplete answer (missing IPv4)");
		return;
	}
	if (port == 0) {
		util_debug(1, "query: incomplete answer (missing port)");
		return;
	}

	if (txt == NULL || txt->txt_cnt == 0) {
		util_append(answer, sizeof(answer), "      \"txt\": [],\n");
	} else {
		util_append(answer, sizeof(answer), "      \"txt\": [\n");
		if (port == 3689) {
			util_append(answer, sizeof(answer), "        \"DAAP (iTunes) Server\",\n");
		}
		for (cnt = 0; cnt < txt->txt_cnt - 1; cnt++) {
			util_append(answer, sizeof(answer), "        \"%s\",\n", txt->txt_data[cnt]);
		}
		util_append(answer, sizeof(answer), "        \"%s\"\n", txt->txt_data[cnt]);
		util_append(answer, sizeof(answer), "      ],\n");
	}

	snprintf(url, sizeof(url), "http://%s:%d/", ipv4, port);
	util_append(answer, sizeof(answer), "      \"url\": \"%s\"\n", url);
	UTIL_STRCAT(answer, "    }");

	for (result = my_results; result != NULL; result = result->next) {
		if (strcmp(result->text, answer) == 0) {
			return;		// duplicate entry
		}
	}

	result = util_malloc(sizeof(result_t));
	result->next = my_results;
	result->text = util_strdup(answer);
	my_results = result;
}


result_t *
query_browse(void)
{
	struct sockaddr_in addr;
	socklen_t addrlen;
	struct ip_mreq mreq;
	int one = 1, timeout, ret;
	struct pollfd fds[1];
	char data[MDNS_SIZE];
	ssize_t len, cnt;

	atexit(query_cleanup);
	timeout = config_get_timeout() * 1000;
	util_info("using mDNS-SD query for discovery (%d ms)", timeout);

	if ((my_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		util_fatal("socket: %s", strerror(errno));
	}
	if (setsockopt(my_sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
		util_fatal("setsockopt(SO_REUSEADDR): %s", strerror(errno));
	}

	addr.sin_family      = AF_INET;
	addr.sin_port        = htons(MDNS_PORT);
	addr.sin_addr.s_addr = inet_addr(INADDR_MDNS);
	addrlen = sizeof(addr);
	if (bind(my_sock, (struct sockaddr *) &addr, addrlen) < 0) {
		util_fatal("bind: %s", strerror(errno));
	}

	mreq.imr_multiaddr.s_addr = inet_addr(INADDR_MDNS);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(my_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		util_fatal("setsockopt(IP_ADD_MEMBERSHIP): %s", strerror(errno));
	} 

	len = (ssize_t) parser_create_query(data, sizeof(data), QUERY_NAME, DNS_RR_TYPE_PTR);
	if (len == 0) {
		util_fatal("%s", parser_get_error());
	}
	util_info("sending mDNS-SD question");

	addr.sin_family      = AF_INET;
	addr.sin_port        = htons(MDNS_PORT);
	addr.sin_addr.s_addr = inet_addr(INADDR_MDNS);
	addrlen = sizeof(addr);
	cnt = sendto(my_sock, data, len, 0, (struct sockaddr *) &addr, addrlen);
	if (cnt != len) {
		util_fatal("can't send query (%s)", strerror(errno));
	}

	for (;;) {
		fds[0].fd = my_sock;
		fds[0].events = POLLIN;
		if ((ret = poll(fds, 1, timeout)) == -1) {
			util_error(__func__, __LINE__, "can't poll mDNS-Sock (%s)", strerror(errno));
			break;
		}
		if (ret == 0) {
			util_info("timeout on mDNS-Sock");
			break;
		}

		if (fds[0].revents & POLLIN) {
			query_read_answer();
		}
	}

	return my_results;
}

