/****************************************************************************
 *
 * Copyright (c) 2017 Volker Wiegand <volker@railduino.de>
 *
 * This file is part of Zeroconf-Lookup.
 *
 *   Zeroconf-Lookup is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Zeroconf-Lookup is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *   See the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with Zeroconf-Lookup.
 *   If not, see <http://www.gnu.org/licenses/>.
 *
 ****************************************************************************/

#include "common.h"
#include "dns-msg-parser.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#define MDNS_SIZE	4096
#define QUERY_NAME	"_http._tcp.local"

#define INADDR_MDNS	"224.0.0.251"
#define MDNS_PORT	5353


static int  mdnssd_sock = 0;
static char answer[MDNS_SIZE];
static result_t *results;


static void
mdnssd_read_answer(int json)
{
	char buf[MDNS_SIZE], url[MDNS_SIZE];
	struct sockaddr_storage addr;
	socklen_t len;
	int cnt, res, num, port;
	DNS_RR rrs[10], *rrp;
	unsigned int length;
	char *ipv4, *ipv6, *name, *txt, *ptr;
	result_t *result;

	len = sizeof(addr);
	cnt = recvfrom(mdnssd_sock, buf, sizeof(buf), 0, (struct sockaddr *) &addr, &len);
	buf[cnt] = '\0';

	if ((res = dns_msg_parse_answer(buf, cnt, 0, rrs, sizeof(rrs))) == -1) {
		util_error("%s", dns_msg_get_error());
		return;
	}
	if (res == 0) {
		//util_debug("got DNS message, but no answer");
		return;
	}

	UTIL_STRCPY(answer, "    {\n");

	for (num = 0, port = 0, ipv4 = ipv6 = name = txt = NULL; num < res; num++) {
		rrp = rrs + num;
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
			if ((ptr = strstr(name, "._ipp._tcp.local")) != NULL) {
				*ptr = '\0';
			}
			if ((ptr = strstr(name, "._workstation._tcp.local")) != NULL) {
				*ptr = '\0';
			}
			util_append(answer, sizeof(answer), "      \"name\": \"%s\",\n", name);
			continue;
		}
		if (rrp->rr_type == DNS_RR_TYPE_TXT) {
			txt = rrp->rr.rr_txt.txt_data;
			util_append(answer, sizeof(answer), "      \"txt\": \"%s\",\n", txt);
			if (strlen(txt) == 0) {
				txt = NULL;
			}
			continue;
		}
		if (rrp->rr_type == DNS_RR_TYPE_AAAA) {
			util_append(answer, sizeof(answer), "      \"aaaa\": \"%s\",\n", rrp->rr.rr_aaaa.aaaa_addr_str);
			if (ipv6 == NULL) {
				ipv6 = rrp->rr.rr_a.a_addr_str;
			}
			continue;
		}
		if (rrp->rr_type == DNS_RR_TYPE_SRV) {
			port = rrp->rr.rr_srv.srv_port;
			util_append(answer, sizeof(answer), "      \"target\": \"%s\",\n", rrp->rr.rr_srv.srv_target);
			util_append(answer, sizeof(answer), "      \"port\": %u,\n",       port);
			util_append(answer, sizeof(answer), "      \"weight\": %u,\n",     rrp->rr.rr_srv.srv_weight);
			util_append(answer, sizeof(answer), "      \"priority\": %u,\n",   rrp->rr.rr_srv.srv_prio);
			continue;
		}
	}

	if (port == 0 || ipv4 == NULL) {
		util_debug("incomplete answer (missing port or IPv4)");
		return;
	}

	snprintf(url, sizeof(url), "http://%s:%d/", ipv4, port);
	util_append(answer, sizeof(answer), "      \"url\": \"%s\"\n", url);
	UTIL_STRCAT(answer, "    }");

	result = util_malloc(sizeof(result_t));
	result->next = results;
	result->text = util_strdup(answer);
	results = result;
}


static void
mdnssd_write_query(void)
{
	char data[MDNS_SIZE];
	size_t len;
	struct sockaddr_in addr;
	socklen_t addrlen;

	len = dns_msg_create_query(data, sizeof(data), 0, QUERY_NAME, DNS_RR_TYPE_PTR);
	if (len == 0) {
		util_error("%s", dns_msg_get_error());
		return;
	}
	util_info("sending mDNS-SD question");

	addr.sin_family      = AF_INET;
	addr.sin_port        = htons(MDNS_PORT);
	addr.sin_addr.s_addr = inet_addr(INADDR_MDNS);
	addrlen = sizeof(addr);

	sendto(mdnssd_sock, data, len, 0, (struct sockaddr *) &addr, addrlen);

	results = NULL;
}


static void
mdnssd_exception(void)
{
	util_error("mdnssd exception ???");
}


void
mdnssd_select(int json, fd_set *rfds, fd_set *wfds, fd_set *xfds)
{
	if (FD_ISSET(mdnssd_sock, rfds)) {
		mdnssd_read_answer(json);
	}

	if (FD_ISSET(mdnssd_sock, wfds)) {
		mdnssd_write_query();
	}

	if (FD_ISSET(mdnssd_sock, xfds)) {
		mdnssd_exception();
	}
}


void
mdnssd_prepare(int query, int *hfd, fd_set *rfds, fd_set *wfds, fd_set *xfds)
{
	struct sockaddr_in addr;
	socklen_t addrlen;
	struct ip_mreq mreq;
	int one = 1;

	if (mdnssd_sock <= 0) {
		mdnssd_sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (mdnssd_sock == -1) {
			err(EXIT_FAILURE, "mdnssd:socket");
		}

		if (setsockopt(mdnssd_sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
			err(EXIT_FAILURE, "mdnssd:SO_REUSEADDR");
		}

		addr.sin_family      = AF_INET;
		addr.sin_port        = htons(MDNS_PORT);
		addr.sin_addr.s_addr = inet_addr(INADDR_MDNS);
		addrlen = sizeof(addr);
		if (bind(mdnssd_sock, (struct sockaddr *) &addr, addrlen) < 0) {
			err(EXIT_FAILURE, "mdnssd:bind");
		}

		mreq.imr_multiaddr.s_addr = inet_addr(INADDR_MDNS);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(mdnssd_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
			err(EXIT_FAILURE, "mdnssd:IP_ADD_MEMBERSHIP");
		} 
	}

	FD_SET(mdnssd_sock, rfds);
	if (query == 1) {
		FD_SET(mdnssd_sock, wfds);
	}
	FD_SET(mdnssd_sock, xfds);
	if (mdnssd_sock > *hfd) {
		*hfd = mdnssd_sock;
	}
}


result_t *
mdnssd_get_result(void)
{
	return results;
}


static void
mdnssd_cleanup(void)
{
	struct ip_mreq mreq;
	result_t *result, *next;

	if (mdnssd_sock > 0) {
		mreq.imr_multiaddr.s_addr = inet_addr(INADDR_MDNS);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		setsockopt(mdnssd_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));

		close(mdnssd_sock);
		mdnssd_sock = 0;
	}

	while (results != NULL) {
		next = results->next;
		util_free(results->text);
		util_free(results);
		results = next;
	}
}


void
mdnssd_init(void)
{
	mdnssd_sock = 0;

	atexit(mdnssd_cleanup);
}

