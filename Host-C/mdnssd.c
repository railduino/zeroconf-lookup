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
#include "dns-msg-parser.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#define MDNS_SIZE	4096
#define QUERY_NAME	"_http._tcp.local"

#define INADDR_MDNS	"224.0.0.251"
#define MDNS_PORT	5353


static int  my_sock = 0;
static char answer[MDNS_SIZE];
static result_t *results;


static void
mdnssd_read_answer(void)
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
	cnt = recvfrom(my_sock, buf, sizeof(buf), 0, (struct sockaddr *) &addr, &len);
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
	port = 0;
	ipv4 = ipv6 = name = NULL;
	txt = "";

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
			txt = rrp->rr.rr_txt.txt_data;		// just remember, will check for iTunes
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

	if (port == 3689) {
		txt = "DAAP (iTunes) Server";
	}
	util_append(answer, sizeof(answer), "      \"txt\": \"%s\",\n", txt);

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

	sendto(my_sock, data, len, 0, (struct sockaddr *) &addr, addrlen);

	results = NULL;
}


static void
mdnssd_exception(void)
{
	util_error("mdnssd exception ???");
}


static void
mdnssd_select(fd_set *rfds, fd_set *wfds, fd_set *xfds)
{
	if (FD_ISSET(my_sock, rfds)) {
		mdnssd_read_answer();
	}

	if (FD_ISSET(my_sock, wfds)) {
		mdnssd_write_query();
	}

	if (FD_ISSET(my_sock, xfds)) {
		mdnssd_exception();
	}
}


static void
mdnssd_prepare(int query, int *hfd, fd_set *rfds, fd_set *wfds, fd_set *xfds)
{
	struct sockaddr_in addr;
	socklen_t addrlen;
	struct ip_mreq mreq;
	int one = 1;

	if (my_sock <= 0) {
		my_sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (my_sock == -1) {
			err(EXIT_FAILURE, "mdnssd:socket");
		}

		if (setsockopt(my_sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
			err(EXIT_FAILURE, "mdnssd:SO_REUSEADDR");
		}

		addr.sin_family      = AF_INET;
		addr.sin_port        = htons(MDNS_PORT);
		addr.sin_addr.s_addr = inet_addr(INADDR_MDNS);
		addrlen = sizeof(addr);
		if (bind(my_sock, (struct sockaddr *) &addr, addrlen) < 0) {
			err(EXIT_FAILURE, "mdnssd:bind");
		}

		mreq.imr_multiaddr.s_addr = inet_addr(INADDR_MDNS);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(my_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
			err(EXIT_FAILURE, "mdnssd:IP_ADD_MEMBERSHIP");
		} 
	}

	FD_SET(my_sock, rfds);
	if (query == 1) {
		FD_SET(my_sock, wfds);
	}
	FD_SET(my_sock, xfds);
	if (my_sock > *hfd) {
		*hfd = my_sock;
	}
}


int
mdnssd_browse(void)
{
	struct timeval timer;
	fd_set rfds, wfds, xfds;
	int query, hfd, res;
	time_t time_up = time(NULL) + config_get_timeout();

	for (query = 1; ; ) {
		if (time(NULL) >= time_up) {
			break;
		}

		timer.tv_sec  = 1;
		timer.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		FD_ZERO(&xfds);

		hfd = 0;
		mdnssd_prepare(query, &hfd, &rfds, &wfds, &xfds);

		if ((res = select(hfd + 1, &rfds, &wfds, &xfds, &timer)) == -1) {
			util_fatal("can't select-mdnssd (%s)", strerror(errno));
		}
		if (res == 0) {
			util_debug("tick-mdnssd...");
			continue;
		}
		query = 0;

		mdnssd_select(&rfds, &wfds, &xfds);
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
	result_t *result;

	if (my_sock > 0) {
		mreq.imr_multiaddr.s_addr = inet_addr(INADDR_MDNS);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		setsockopt(my_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));

		close(my_sock);
		my_sock = 0;
	}

	while (results != NULL) {
		result = results->next;
		util_free(results->text);
		util_free(results);
		results = result;
	}
}


void
mdnssd_init(void)
{
	my_sock = 0;
	results = NULL;

	atexit(mdnssd_cleanup);
}

