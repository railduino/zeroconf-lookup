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

/**
 * @file parser.h
 * @author Volker Wiegand
 * @date 17-DEC-2017
 * @brief DNS Format Structures and Prototypes for mDNS-SD
 *
 * @see https://tools.ietf.org/html/rfc1035#section-4
 * @see https://tools.ietf.org/html/rfc2782 (PTR)
 * @see https://tools.ietf.org/html/rfc3596 (AAAA)
 * @see https://en.wikipedia.org/wiki/List_of_DNS_record_types
 */

#ifndef _PARSER_H
#define _PARSER_H 1


#include <arpa/inet.h>	// for in_addr


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

#define TXT_MAX			10


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
	int		txt_cnt;
	char		txt_data[TXT_MAX][DNS_NAME_SIZE];
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


/**
 * Function prototypes
 */

char *parser_get_error(void);
size_t parser_create_query(char *data, size_t len, char *name, uint16_t qtype);
int parser_parse_answer(char *data, size_t len, DNS_RR *rr, int rr_size);

#endif /* !_PARSER_H */

