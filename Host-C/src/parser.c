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

#if defined(HAVE_QUERY)

#include "parser.h"


/**
 *
 */

static char err_buf[4096];

static size_t label_ofs[256];
static size_t label_cnt;

static uint16_t query_id = 0;


static void
parser_set_error(const char *func, char *fmt, ...)
{
	size_t ofs;
	va_list ap;

	snprintf(err_buf, sizeof(err_buf), "%s: ", func);
	ofs = strlen(err_buf);

	va_start(ap, fmt);
	vsnprintf(err_buf + ofs, sizeof(err_buf) - ofs, fmt, ap);
	va_end(ap);
}


char *
parser_get_error(void)
{
	return err_buf;
}


size_t
parser_create_query(char *data, size_t len, char *name, uint16_t qtype)
{
	DNS_HEADER *hdr;
	DNS_QUESTION que;
	char buf[DNS_NAME_SIZE], *src;
	size_t ofs, siz;

	memset(err_buf, '\0', sizeof(err_buf));

	if (name == NULL || strlen(name) == 0) {
		parser_set_error(__func__, "missing name");
		return 0;
	}
	if (strlen(name) > (DNS_NAME_SIZE) - 2) {
		parser_set_error(__func__, "name too long");
		return 0;
	}
	if (len < (sizeof(DNS_HEADER) + strlen(name) + 6)) {
		parser_set_error(__func__, "buffer too small");
		return 0;
	}
	memset(data, '\0', len);
	query_id++;

	hdr = (DNS_HEADER *) data;
	hdr->msg_id      = htons(query_id);
	hdr->msg_flags   = 0;
	hdr->msg_qdcount = htons(1);
	hdr->msg_ancount = htons(0);
	hdr->msg_nscount = htons(0);
	hdr->msg_arcount = htons(0);

	ofs = sizeof(DNS_HEADER);
	strncpy(buf, name, sizeof(buf));
	for (src = strtok(buf, "."); src != NULL; src = strtok(NULL, ".")) {
		siz = strlen(src);
		data[ofs++] = (char) siz;
		memcpy(data + ofs, src, siz);
		ofs += siz;
	}
	data[ofs++] = '\0';

	que.que_qtype  = htons(qtype);
	que.que_qclass = htons(DNS_CLASS_IN);
	memcpy(data + ofs, &que, sizeof(DNS_QUESTION));

	return ofs + sizeof(DNS_QUESTION);
}


/**
 *
 */

static size_t
parser_parse_name(char *data, size_t len, size_t start, char *dst)
{
	uint16_t jmp;
	char *ptr;
	size_t ofs, siz, final, num;

	for (ptr = dst, ofs = start, final = 0; ; ) {
		memcpy(&jmp, data + ofs, sizeof(jmp));
		jmp = ntohs(jmp);
		if ((jmp & 0xc000) == 0xc000) {
			if (final == 0) {
				final = ofs + sizeof(jmp);
			}
			ofs = jmp & 0x3fff;
			for (num = 0; num < label_cnt; num++) {
				if (ofs == label_ofs[num]) {
					break;
				}
			}
			if (num >= label_cnt) {
				parser_set_error(__func__, "invalid jump target");
				return 0;
			}
			continue;
		}

		if ((siz = (size_t) data[ofs]) == 0) {
			return final ? final : (ofs + 1);
		}

		if (siz > 63) {
			parser_set_error(__func__, "label is too long");
			return 0;
		}
		label_ofs[label_cnt++] = ofs;

		if ((ofs + siz + 1) > len) {
			parser_set_error(__func__, "name is too long");
			return 0;
		}

		if (ptr != dst) {
			*ptr++ = '.';
		}
		memcpy(ptr, data + ofs + 1, siz);
		ptr += siz;
		ofs += (siz + 1);
	}

	parser_set_error(__func__, "unfinished name");
	return 0;
}


/**
 *
 */

static size_t
parser_parse_rr_a(char *data, size_t len, size_t start, DNS_RR *rr)
{
	size_t adr_len = sizeof(rr->rr.rr_a.a_addr);
	size_t str_len = sizeof(rr->rr.rr_a.a_addr_str);

	if ((start + adr_len) > len) {
		parser_set_error(__func__, "buffer len too small for A record");
		return 0;
	}

	memcpy(&(rr->rr.rr_a.a_addr), data + start, adr_len);
	inet_ntop(AF_INET, &(rr->rr.rr_a.a_addr), rr->rr.rr_a.a_addr_str, str_len);

	return start + adr_len;
}


/**
 *
 */

static size_t
parser_parse_rr_ptr(char *data, size_t len, size_t start, DNS_RR *rr)
{
	return parser_parse_name(data, len, start, rr->rr.rr_ptr.ptr_dname);
}


/**
 *
 */

static size_t
parser_parse_rr_txt(char *data, size_t len, size_t start, DNS_RR *rr)
{
	size_t siz = (size_t) data[start];

	if ((start + siz) > len) {
		parser_set_error(__func__, "name is too long");
		return 0;
	}

	memcpy(rr->rr.rr_txt.txt_data, data + start + 1, siz);

	return start + siz + 1;
}


/**
 *
 */

static size_t
parser_parse_rr_aaaa(char *data, size_t len, size_t start, DNS_RR *rr)
{
	size_t adr_len = sizeof(rr->rr.rr_aaaa.aaaa_addr);
	size_t str_len = sizeof(rr->rr.rr_aaaa.aaaa_addr_str);

	if ((start + adr_len) > len) {
		parser_set_error(__func__, "buffer len too small for AAAA record");
		return 0;
	}

	memcpy(&(rr->rr.rr_aaaa.aaaa_addr), data + start, adr_len);
	inet_ntop(AF_INET6, &(rr->rr.rr_aaaa.aaaa_addr), rr->rr.rr_aaaa.aaaa_addr_str, str_len);

	return start + adr_len;
}


/**
 *
 */

static size_t
parser_parse_rr_srv(char *data, size_t len, size_t start, DNS_RR *rr)
{
	memcpy(&(rr->rr.rr_srv.srv_prio), data + start, sizeof(rr->rr.rr_srv.srv_prio));
	rr->rr.rr_srv.srv_prio = ntohs(rr->rr.rr_srv.srv_prio);
	start += sizeof(rr->rr.rr_srv.srv_prio);

	memcpy(&(rr->rr.rr_srv.srv_weight), data + start, sizeof(rr->rr.rr_srv.srv_weight));
	rr->rr.rr_srv.srv_weight = ntohs(rr->rr.rr_srv.srv_weight);
	start += sizeof(rr->rr.rr_srv.srv_weight);

	memcpy(&(rr->rr.rr_srv.srv_port), data + start, sizeof(rr->rr.rr_srv.srv_port));
	rr->rr.rr_srv.srv_port = ntohs(rr->rr.rr_srv.srv_port);
	start += sizeof(rr->rr.rr_srv.srv_port);

	return parser_parse_name(data, len, start, rr->rr.rr_srv.srv_target);
}


/**
 *
 */

int
parser_parse_answer(char *data, size_t len, DNS_RR *rr, int rr_size)
{
	DNS_HEADER hdr;
	int num, cnt;
	size_t start;

	memset(err_buf, '\0', sizeof(err_buf));
	label_cnt = 0;

	if (len < sizeof(DNS_HEADER)) {
		parser_set_error(__func__, "buffer len too small for header");
		return -1;
	}
	memcpy(&hdr, data, sizeof(DNS_HEADER));
	hdr.msg_id      = ntohs(hdr.msg_id);
	hdr.msg_flags   = ntohs(hdr.msg_flags);
	hdr.msg_qdcount = ntohs(hdr.msg_qdcount);
	hdr.msg_ancount = ntohs(hdr.msg_ancount);
	hdr.msg_nscount = ntohs(hdr.msg_nscount);
	hdr.msg_arcount = ntohs(hdr.msg_arcount);

	if ((hdr.msg_flags & 0x8000) != 0x8000) {
		return 0;
	}
	if ((hdr.msg_flags & 0x000f) != 0x0000) {
		parser_set_error(__func__, "error code: %d", (hdr.msg_flags & 0x000f));
		return -1;
	}
	if (hdr.msg_nscount > 0) {
		parser_set_error(__func__, "answer contains %u NS records", hdr.msg_nscount);
		return -1;
	}
	if (hdr.msg_id != query_id) {
		parser_set_error(__func__, "answer-ID %u does not match query-ID %u", hdr.msg_id, query_id);
		// ignore this mismatch, since it seems not to be critical
	}
	cnt = hdr.msg_ancount + hdr.msg_arcount;
	if (rr_size < cnt) {
		parser_set_error(__func__, "rr_size too small (need %d)", cnt);
		return -1;
	}
	start = sizeof(DNS_HEADER);

	for (num = 0; num < cnt; num++, rr++) {
		memset(rr, '\0', sizeof(DNS_RR));

		if ((start = parser_parse_name(data, len, start, rr->rr_name)) == 0) {
			return -1;
		}

		memcpy(&(rr->rr_type), data + start, sizeof(rr->rr_type));
		rr->rr_type = ntohs(rr->rr_type);
		start += sizeof(rr->rr_type);

		memcpy(&(rr->rr_class), data + start, sizeof(rr->rr_class));
		rr->rr_class = ntohs(rr->rr_class);
		start += sizeof(rr->rr_class);

		memcpy(&(rr->rr_ttl), data + start, sizeof(rr->rr_ttl));
		rr->rr_ttl = ntohl(rr->rr_ttl);
		start += sizeof(rr->rr_ttl);

		memcpy(&(rr->rr_rdlength), data + start, sizeof(rr->rr_rdlength));
		rr->rr_rdlength = ntohs(rr->rr_rdlength);
		start += sizeof(rr->rr_rdlength);

		switch (rr->rr_type) {
			case DNS_RR_TYPE_A:
				if (parser_parse_rr_a(data, len, start, rr) == 0) {
					return -1;
				}
				break;
			case DNS_RR_TYPE_PTR:
				if (parser_parse_rr_ptr(data, len, start, rr) == 0) {
					return -1;
				}
				break;
			case DNS_RR_TYPE_TXT:
				if (parser_parse_rr_txt(data, len, start, rr) == 0) {
					return -1;
				}
				break;
			case DNS_RR_TYPE_AAAA:
				if (parser_parse_rr_aaaa(data, len, start, rr) == 0) {
					return -1;
				}
				break;
			case DNS_RR_TYPE_SRV:
				if (parser_parse_rr_srv(data, len, start, rr) == 0) {
					return -1;
				}
				break;
			default:
				parser_set_error(__func__, "invalid RR-Type 0x%02x", rr->rr_type);
				return -1;
		}

		start += rr->rr_rdlength;
	}

	return num;
}

#endif /* defined(HAVE_QUERY) */

