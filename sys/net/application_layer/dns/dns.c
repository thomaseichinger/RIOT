/*
 * Copyright (c) 2017 Thomas Eichinger <thomas@riot-os.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */


#include <string.h>

#include "net/sock/udp.h"
#include "net/dns.h"

ssize_t _encode_domain_name(uint8_t *out, const char *domain_name)
{
    uint8_t *part_start = out;
    uint8_t *out_pos = ++out;

    char c;

    while ((c = *domain_name)) {
        if (c == '.') {
            *part_start = (out_pos - part_start - 1);
            part_start = out_pos++;
        }
        else {
            *out_pos++ = c;
        }
        domain_name++;
    }

    *part_start = (out_pos - part_start - 1);
    *out_pos++ = 0;

    return out_pos - out +1;
}

static unsigned int _put_short(uint8_t *out, uint16_t val)
{
    *((uint16_t*)(&(out[1]))) = val;
    return 2;
}

static uint16_t _get_short(uint8_t *in)
{
    return (uint16_t)((in[0]<<8)|in[1]);
}

size_t _skip_hostname(uint8_t *buffer)
{
    /* check for DNS msg compresssion */
    if (*buffer >= 192) {
        return 2;
    }
    uint8_t *_tmp = buffer;
    for (; (*_tmp) != 0; _tmp += (*_tmp) +1) {}
    return (_tmp - buffer + 1);
}

int _parse_dns_reply(uint8_t *buffer, size_t len, void* addr_out, int addr_family)
{
    dns_pkt_t *pkt = (dns_pkt_t*) buffer;
    uint8_t *_tmp = buffer + sizeof(*pkt);

    /* skip all queries that are part of the reply */
    for (unsigned n = 0; n < ntohs(pkt->q_count); n++) {
        _tmp += _skip_hostname(_tmp);
        _tmp += 4;    /* skip type and class of query */
    }

    for (unsigned n = 0; n < ntohs(pkt->a_count); n++) {
        _tmp += _skip_hostname(_tmp);
        uint16_t _type = ntohs(_get_short(_tmp));
        _tmp += 2;
        uint16_t class = ntohs(_get_short(_tmp));
        _tmp += 2;
        _tmp += 4; /* skip ttl */

        unsigned addrlen = ntohs(_get_short(_tmp));
        _tmp += 2;
        if ((_tmp + addrlen) > (buffer + len)) {
            return -EBADMSG;
        }

        /* skip unwanted answers */
        if ((class != DNS_CLASS_IN) ||
            ((_type == DNS_TYPE_A) && (addr_family == AF_INET6)) ||
            ((_type == DNS_TYPE_AAAA) && (addr_family == AF_INET)) ||
            !((_type == DNS_TYPE_A) || (_type == DNS_TYPE_AAAA))) {

            _tmp += addrlen;
            continue;
        }

        memcpy(addr_out, _tmp, addrlen);
        return addrlen;
    }

    return -1;
}

int dns_query(const char *name, void *addr_out, int addr_family)
{
    uint8_t buffer[DNS_MAX_QUERY_BUFFER_LEN];
    uint8_t reply_buffer[DNS_MAX_REPLY_BUFFER_LEN];

    dns_pkt_t *pkt = (dns_pkt_t*) buffer;
    /* clear memory */
    memset(pkt, 0, sizeof(*pkt));
    /* use thread's ID as query ID */
    pkt->id = htons(thread_getpid());
    /* set query flags
     * TODO: use defines instead of magic number */
    pkt->flags = htons(0x0120);
    pkt->q_count = htons(1 + (addr_family == AF_UNSPEC));

    uint8_t *buffer_pos = buffer + sizeof(*pkt);

    unsigned _name_ptr;
    if ((addr_family == AF_INET6) || (addr_family == AF_UNSPEC)) {
        _name_ptr = (buffer_pos - buffer);
        buffer_pos += _encode_domain_name(buffer_pos, name);
        buffer_pos += _put_short(buffer_pos, htons(DNS_TYPE_AAAA));
        buffer_pos += _put_short(buffer_pos, htons(DNS_CLASS_IN));
    }

    if ((addr_family == AF_INET) || (addr_family == AF_UNSPEC)) {
        if (addr_family == AF_UNSPEC) {
            buffer_pos += _put_short(buffer_pos, htons((0xc000) | (_name_ptr)));
        }
        else {
            buffer_pos += _encode_domain_name(buffer_pos, name);
        }
        buffer_pos += _put_short(buffer_pos, htons(DNS_TYPE_A));
        buffer_pos += _put_short(buffer_pos, htons(DNS_CLASS_IN));
    }

    uint8_t _addrtmp[4] = {8,8,8,8};
    sock_udp_ep_t dns_ep = { .family=AF_INET, .port=DNS_SERV_PORT };
    memcpy(&dns_ep.addr.ipv4, _addrtmp, 4);

    sock_udp_t sock_dns;

    ssize_t res = sock_udp_create(&sock_dns, NULL, &dns_ep, 0);
    
    if (!res) {
        // for (int i = 0; i < SOCK_DNS_RETRIES; i++) {
        res = sock_udp_send(&sock_dns, buffer, (buffer_pos - buffer), NULL);
        if (res <= 0) {
            sock_udp_close(&sock_dns);
            return res;
        }
        res = sock_udp_recv(&sock_dns, reply_buffer, sizeof(reply_buffer), 1000000LU, NULL);
        if (res > DNS_MIN_REPLY_LEN) {
            if ((res = _parse_dns_reply(reply_buffer, res, addr_out, addr_family)) > 0) {
                sock_udp_close(&sock_dns);
                return res;
            }
        }
            // break;
        // }
    }

    sock_udp_close(&sock_dns);
    return res;
}
