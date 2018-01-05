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

/**
 * @file dns-msg-parser.h
 * @author Volker Wiegand
 * @date 17-DEC-2017
 * @brief DNS Format Structures and Prototypes for mDNS-SD
 *
 * @see https://tools.ietf.org/html/rfc1035#section-4
 * @see https://tools.ietf.org/html/rfc2782 (PTR)
 * @see https://tools.ietf.org/html/rfc3596 (AAAA)
 * @see https://en.wikipedia.org/wiki/List_of_DNS_record_types
 */


#ifndef _DNS_MSG_PARSER_H
#define _DNS_MSG_PARSER_H 1


#include <netdb.h>		// for uint16_t and uint32_t
#include <arpa/inet.h>		// for in_addr


#define DNS_NAME_SIZE		256

#define DNS_CLASS_IN		1

#define DNS_RR_TYPE_A		1
#define DNS_RR_TYPE_NS		2
#define DNS_RR_TYPE_CNAME	5
#define DNS_RR_TYPE_SOA		6
#define DNS_RR_TYPE_PTR		12
#define DNS_RR_TYPE_MX		15
#define DNS_RR_TYPE_TXT		16
#define DNS_RR_TYPE_AAAA	28
#define DNS_RR_TYPE_SRV		33


/**
 * @brief DNS_HEADER
 *
 * The DNS message header is common to all DNS messages.
 * All numeric fields are in network order.
 *
 * @see https://tools.ietf.org/html/rfc1035#section-4.1
 */

typedef struct {
	uint16_t	msg_id;		///< Each message carries an ID
	uint16_t	msg_flags;
	uint16_t	msg_qdcount;	///< number of entries in the question section
	uint16_t	msg_ancount;	///< number of resource records in the answer section
	uint16_t	msg_nscount;	///< number of name server resource records in the authority records section
	uint16_t	msg_arcount;	///< number of resource records in the additional records section
} DNS_HEADER;


/**
 *
 */

typedef struct {
	uint16_t	que_qtype;
	uint16_t	que_qclass;
} DNS_QUESTION;


/**
 *
 */

typedef struct {
	struct in_addr	a_addr;
	char		a_addr_str[INET_ADDRSTRLEN];
} DNS_RR_A;


/**
 *
 */

typedef struct {
	char		ptr_dname[DNS_NAME_SIZE];
} DNS_RR_PTR;


/**
 *
 */

typedef struct {
	char		txt_data[DNS_NAME_SIZE];
} DNS_RR_TXT;


/**
 *
 */

typedef struct {
	struct in6_addr	aaaa_addr;
	char		aaaa_addr_str[INET6_ADDRSTRLEN];
} DNS_RR_AAAA;


/**
 *
 */

typedef struct {
	uint16_t	srv_prio;
	uint16_t	srv_weight;
	uint16_t	srv_port;
	char		srv_target[DNS_NAME_SIZE];
} DNS_RR_SRV;


/**
 *
 */

typedef struct {
	char		rr_name[DNS_NAME_SIZE];
	uint16_t	rr_type;
	uint16_t	rr_class;
	uint32_t	rr_ttl;
	uint16_t	rr_rdlength;
	union {
		char		rr_rdata[DNS_NAME_SIZE];
		DNS_RR_A	rr_a;
		DNS_RR_PTR	rr_ptr;
		DNS_RR_TXT	rr_txt;
		DNS_RR_AAAA	rr_aaaa;
		DNS_RR_SRV	rr_srv;
	} rr;
} DNS_RR;


char *dns_msg_get_error(void);

size_t dns_msg_create_query(char *data, size_t len, uint16_t id, char *name, uint16_t qtype);

int dns_msg_parse_answer(char *data, size_t len, uint16_t id, DNS_RR *rr, int rr_size);


#endif /* !_DNS_MSG_PARSER_H */

