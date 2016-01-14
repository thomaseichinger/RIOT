/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 */
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include "embUnit/embUnit.h"

#include "byteorder.h"
#include "net/ieee802154.h"

#include "unittests-constants.h"
#include "tests-ieee802154.h"

static void test_ieee802154_set_frame_hdr_flags0(void)
{
    const uint8_t exp[] = { 0x00, 0x00, TEST_UINT8 };
    uint8_t res[sizeof(exp)];
    const network_uint16_t flags = byteorder_htons(0x0000);

    TEST_ASSERT_EQUAL_INT(sizeof(exp), ieee802154_set_frame_hdr(res, NULL, NULL,
                                                                flags, TEST_UINT8));
    TEST_ASSERT_EQUAL_INT(0, memcmp(exp, res, sizeof(exp)));
}

static void test_ieee802154_set_frame_hdr_flags0_non_beacon_non_ack(void)
{
    uint8_t res;
    const network_uint16_t flags = byteorder_htons(0x0100);

    TEST_ASSERT_EQUAL_INT(0, ieee802154_set_frame_hdr(&res, NULL, NULL,
                                                      flags, TEST_UINT8));
}

static void test_ieee802154_set_frame_hdr_reserved_dst_mode(void)
{
    uint8_t res[3];
    const network_uint16_t flags = byteorder_htons(0x0004);

    TEST_ASSERT_EQUAL_INT(0, ieee802154_set_frame_hdr(res, NULL, NULL,
                                                      flags, TEST_UINT8));
}

static void test_ieee802154_set_frame_hdr_reserved_src_mode(void)
{
    uint8_t res[3];
    const network_uint16_t flags = byteorder_htons(0x0040);

    TEST_ASSERT_EQUAL_INT(0, ieee802154_set_frame_hdr(res, NULL, NULL,
                                                      flags, TEST_UINT8));
}

static void test_ieee802154_set_frame_hdr_dst_2_src_0_dst_NULL(void)
{
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16);
    const uint8_t exp[] = { 0x00, 0x08, TEST_UINT8, exp_dst_pan.u8[1],
                            exp_dst_pan.u8[0], 0xff, 0xff };
    uint8_t res[sizeof(exp)];
    const network_uint16_t flags = byteorder_htons(0x0008);
    const ieee802154_addr_t src = { exp_dst_pan, { byteorder_htonll(TEST_UINT64) } };

    TEST_ASSERT_EQUAL_INT(sizeof(exp), ieee802154_set_frame_hdr(res, &src, NULL,
                                                                flags, TEST_UINT8));
    TEST_ASSERT_EQUAL_INT(0, memcmp(exp, res, sizeof(exp)));
}

static void test_ieee802154_set_frame_hdr_dst_2_src_0_dst_NOT_NULL(void)
{
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    const uint8_t exp[] = { 0x00, 0x08, TEST_UINT8, exp_dst_pan.u8[1],
                            exp_dst_pan.u8[0], exp_dst_addr.u8[1],
                            exp_dst_addr.u8[0] };
    uint8_t res[sizeof(exp)];
    const network_uint16_t flags = byteorder_htons(0x0008);
    const ieee802154_addr_t src = { byteorder_htons(TEST_UINT16),
                                    { byteorder_htonll(TEST_UINT64) } };
    const ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };

    TEST_ASSERT_EQUAL_INT(sizeof(exp), ieee802154_set_frame_hdr(res, &src, &dst,
                                                                flags, TEST_UINT8));
    TEST_ASSERT_EQUAL_INT(0, memcmp(exp, res, sizeof(exp)));
}

static void test_ieee802154_set_frame_hdr_dst_3_src_0_dst_NULL(void)
{
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16);
    const uint8_t exp[] = { 0x00, 0x08, TEST_UINT8, exp_dst_pan.u8[1],
                            exp_dst_pan.u8[0], 0xff, 0xff };
    uint8_t res[sizeof(exp)];
    const network_uint16_t flags = byteorder_htons(0x0008);
    const ieee802154_addr_t src = { exp_dst_pan, { byteorder_htonll(TEST_UINT64) } };

    TEST_ASSERT_EQUAL_INT(sizeof(exp), ieee802154_set_frame_hdr(res, &src, NULL,
                                                                flags, TEST_UINT8));
    TEST_ASSERT_EQUAL_INT(0, memcmp(exp, res, sizeof(exp)));
}

static void test_ieee802154_set_frame_hdr_dst_3_src_0_dst_NOT_NULL(void)
{
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    const uint8_t exp[] = { 0x00, 0x0c, TEST_UINT8,
                            exp_dst_pan.u8[1], exp_dst_pan.u8[0],
                            exp_dst_addr.u8[7], exp_dst_addr.u8[6],
                            exp_dst_addr.u8[5], exp_dst_addr.u8[4],
                            exp_dst_addr.u8[3], exp_dst_addr.u8[2],
                            exp_dst_addr.u8[1], exp_dst_addr.u8[0] };
    uint8_t res[sizeof(exp)];
    const network_uint16_t flags = byteorder_htons(0x000c);
    const ieee802154_addr_t src = { byteorder_htons(TEST_UINT16),
                                    { byteorder_htonll(TEST_UINT64) } };
    const ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };

    TEST_ASSERT_EQUAL_INT(sizeof(exp), ieee802154_set_frame_hdr(res, &src, &dst,
                                                                flags, TEST_UINT8));
    TEST_ASSERT_EQUAL_INT(0, memcmp(exp, res, sizeof(exp)));
}

static void test_ieee802154_set_frame_hdr_flags0_ver_1(void)
{
    const uint8_t exp[] = { 0x00, 0x10, TEST_UINT8 };
    uint8_t res[sizeof(exp)];
    const network_uint16_t flags = byteorder_htons(0x0010);

    TEST_ASSERT_EQUAL_INT(sizeof(exp), ieee802154_set_frame_hdr(res, NULL, NULL,
                                                                flags, TEST_UINT8));
    TEST_ASSERT_EQUAL_INT(0, memcmp(exp, res, sizeof(exp)));
}

static void test_ieee802154_set_frame_hdr_dst_0_src_2(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const uint8_t exp[] = { 0x00, 0x90, TEST_UINT8,
                            exp_src_pan.u8[1], exp_src_pan.u8[0],
                            exp_src_addr.u8[1], exp_src_addr.u8[0] };
    uint8_t res[sizeof(exp)];
    const network_uint16_t flags = byteorder_htons(0x0090);
    const ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };

    TEST_ASSERT_EQUAL_INT(sizeof(exp), ieee802154_set_frame_hdr(res, &src, NULL,
                                                                flags, TEST_UINT8));
    TEST_ASSERT_EQUAL_INT(0, memcmp(exp, res, sizeof(exp)));
}

static void test_ieee802154_set_frame_hdr_dst_0_src_3(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const uint8_t exp[] = { 0x00, 0xd0, TEST_UINT8,
                            exp_src_pan.u8[1], exp_src_pan.u8[0],
                            exp_src_addr.u8[7], exp_src_addr.u8[6],
                            exp_src_addr.u8[5], exp_src_addr.u8[4],
                            exp_src_addr.u8[3], exp_src_addr.u8[2],
                            exp_src_addr.u8[1], exp_src_addr.u8[0] };
    uint8_t res[sizeof(exp)];
    const network_uint16_t flags = byteorder_htons(0x00d0);
    const ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };

    TEST_ASSERT_EQUAL_INT(sizeof(exp), ieee802154_set_frame_hdr(res, &src, NULL,
                                                                flags, TEST_UINT8));
    TEST_ASSERT_EQUAL_INT(0, memcmp(exp, res, sizeof(exp)));
}

static void test_ieee802154_set_frame_hdr_dst_2_src_2_autopan_compr(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    const uint8_t exp[] = { 0x40, 0x98, TEST_UINT8,
                            exp_dst_pan.u8[1], exp_dst_pan.u8[0],
                            exp_dst_addr.u8[1], exp_dst_addr.u8[0],
                            exp_src_addr.u8[1], exp_src_addr.u8[0] };
    uint8_t res[sizeof(exp)];
    const network_uint16_t flags = byteorder_htons(0x4098);
    const ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    const ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };

    TEST_ASSERT_EQUAL_INT(sizeof(exp), ieee802154_set_frame_hdr(res, &src, &dst,
                                                                flags, TEST_UINT8));
    TEST_ASSERT_EQUAL_INT(0, memcmp(exp, res, sizeof(exp)));
}

static void test_ieee802154_set_frame_hdr_dst_2_src_2(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    const uint8_t exp[] = { 0x02, 0x98, TEST_UINT8,
                            exp_dst_pan.u8[1], exp_dst_pan.u8[0],
                            exp_dst_addr.u8[1], exp_dst_addr.u8[0],
                            exp_src_pan.u8[1], exp_src_pan.u8[0],
                            exp_src_addr.u8[1], exp_src_addr.u8[0] };
    uint8_t res[sizeof(exp)];
    const network_uint16_t flags = byteorder_htons(0x0298);
    const ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    const ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };

    TEST_ASSERT_EQUAL_INT(sizeof(exp), ieee802154_set_frame_hdr(res, &src, &dst,
                                                                flags, TEST_UINT8));
    TEST_ASSERT_EQUAL_INT(0, memcmp(exp, res, sizeof(exp)));
}

static void test_ieee802154_set_frame_hdr_dst_3_src_2(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    const uint8_t exp[] = { 0x01, 0x9c, TEST_UINT8,
                            exp_dst_pan.u8[1], exp_dst_pan.u8[0],
                            exp_dst_addr.u8[7], exp_dst_addr.u8[6],
                            exp_dst_addr.u8[5], exp_dst_addr.u8[4],
                            exp_dst_addr.u8[3], exp_dst_addr.u8[2],
                            exp_dst_addr.u8[1], exp_dst_addr.u8[0],
                            exp_src_pan.u8[1], exp_src_pan.u8[0],
                            exp_src_addr.u8[1], exp_src_addr.u8[0] };
    uint8_t res[sizeof(exp)];
    const network_uint16_t flags = byteorder_htons(0x019c);
    const ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    const ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };

    TEST_ASSERT_EQUAL_INT(sizeof(exp), ieee802154_set_frame_hdr(res, &src, &dst,
                                                                flags, TEST_UINT8));
    TEST_ASSERT_EQUAL_INT(0, memcmp(exp, res, sizeof(exp)));
}

static void test_ieee802154_set_frame_hdr_dst_2_src_3(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    const uint8_t exp[] = { 0x03, 0xd8, TEST_UINT8,
                            exp_dst_pan.u8[1], exp_dst_pan.u8[0],
                            exp_dst_addr.u8[1], exp_dst_addr.u8[0],
                            exp_src_pan.u8[1], exp_src_pan.u8[0],
                            exp_src_addr.u8[7], exp_src_addr.u8[6],
                            exp_src_addr.u8[5], exp_src_addr.u8[4],
                            exp_src_addr.u8[3], exp_src_addr.u8[2],
                            exp_src_addr.u8[1], exp_src_addr.u8[0] };
    uint8_t res[sizeof(exp)];
    const network_uint16_t flags = byteorder_htons(0x03d8);
    const ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    const ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };

    TEST_ASSERT_EQUAL_INT(sizeof(exp), ieee802154_set_frame_hdr(res, &src, &dst,
                                                                flags, TEST_UINT8));
    TEST_ASSERT_EQUAL_INT(0, memcmp(exp, res, sizeof(exp)));
}

static void test_ieee802154_set_frame_hdr_dst_3_src_3(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    const uint8_t exp[] = { 0x02, 0xdc, TEST_UINT8,
                            exp_dst_pan.u8[1], exp_dst_pan.u8[0],
                            exp_dst_addr.u8[7], exp_dst_addr.u8[6],
                            exp_dst_addr.u8[5], exp_dst_addr.u8[4],
                            exp_dst_addr.u8[3], exp_dst_addr.u8[2],
                            exp_dst_addr.u8[1], exp_dst_addr.u8[0],
                            exp_src_pan.u8[1], exp_src_pan.u8[0],
                            exp_src_addr.u8[7], exp_src_addr.u8[6],
                            exp_src_addr.u8[5], exp_src_addr.u8[4],
                            exp_src_addr.u8[3], exp_src_addr.u8[2],
                            exp_src_addr.u8[1], exp_src_addr.u8[0] };
    uint8_t res[sizeof(exp)];
    const network_uint16_t flags = byteorder_htons(0x02dc);
    const ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    const ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };

    TEST_ASSERT_EQUAL_INT(sizeof(exp), ieee802154_set_frame_hdr(res, &src, &dst,
                                                                flags, TEST_UINT8));
    TEST_ASSERT_EQUAL_INT(0, memcmp(exp, res, sizeof(exp)));
}

static void test_ieee802154_set_frame_hdr_dst_3_src_3_pan_comp(void)
{
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    const uint8_t exp[] = { 0x41, 0xdc, TEST_UINT8,
                            exp_dst_pan.u8[1], exp_dst_pan.u8[0],
                            exp_dst_addr.u8[7], exp_dst_addr.u8[6],
                            exp_dst_addr.u8[5], exp_dst_addr.u8[4],
                            exp_dst_addr.u8[3], exp_dst_addr.u8[2],
                            exp_dst_addr.u8[1], exp_dst_addr.u8[0],
                            exp_src_addr.u8[7], exp_src_addr.u8[6],
                            exp_src_addr.u8[5], exp_src_addr.u8[4],
                            exp_src_addr.u8[3], exp_src_addr.u8[2],
                            exp_src_addr.u8[1], exp_src_addr.u8[0] };
    uint8_t res[sizeof(exp)];
    const network_uint16_t flags = byteorder_htons(0x41dc);
    const ieee802154_addr_t src = { byteorder_htons(0), { exp_src_addr } };
    const ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };

    TEST_ASSERT_EQUAL_INT(sizeof(exp), ieee802154_set_frame_hdr(res, &src, &dst,
                                                                flags, TEST_UINT8));
    TEST_ASSERT_EQUAL_INT(0, memcmp(exp, res, sizeof(exp)));
}

static void test_ieee802154_get_frame_hdr_len_src_mode(void)
{
    const uint8_t mhr[] = { 0x00, 0x00 };

    TEST_ASSERT_EQUAL_INT(3, ieee802154_get_frame_hdr_len(mhr));
}

static void test_ieee802154_get_frame_hdr_len_reserved_dst_mode(void)
{
    const uint8_t mhr[] = { 0x00, 0x04 };

    TEST_ASSERT_EQUAL_INT(0, ieee802154_get_frame_hdr_len(mhr));
}

static void test_ieee802154_get_frame_hdr_len_reserved_src_mode(void)
{
    const uint8_t mhr[] = { 0x00, 0x40 };

    TEST_ASSERT_EQUAL_INT(0, ieee802154_get_frame_hdr_len(mhr));
}

static void test_ieee802154_get_frame_hdr_len_dst_2_src_0(void)
{
    const uint8_t mhr[] = { 0x00, 0x08 };

    TEST_ASSERT_EQUAL_INT(7, ieee802154_get_frame_hdr_len(mhr));
}

static void test_ieee802154_get_frame_hdr_len_dst_3_src_0(void)
{
    const uint8_t mhr[] = { 0x00, 0x0c };

    TEST_ASSERT_EQUAL_INT(13, ieee802154_get_frame_hdr_len(mhr));
}

static void test_ieee802154_get_frame_hdr_len_dst_0_src_2(void)
{
    const uint8_t mhr[] = { 0x00, 0x80 };

    TEST_ASSERT_EQUAL_INT(7, ieee802154_get_frame_hdr_len(mhr));
}

static void test_ieee802154_get_frame_hdr_len_dst_0_src_2_pan_comp(void)
{
    const uint8_t mhr[] = { 0x40, 0x80 };

    TEST_ASSERT_EQUAL_INT(0, ieee802154_get_frame_hdr_len(mhr));
}

static void test_ieee802154_get_frame_hdr_len_dst_0_src_3(void)
{
    const uint8_t mhr[] = { 0x00, 0xc0 };

    TEST_ASSERT_EQUAL_INT(13, ieee802154_get_frame_hdr_len(mhr));
}

static void test_ieee802154_get_frame_hdr_len_dst_0_src_3_pan_comp(void)
{
    const uint8_t mhr[] = { 0x40, 0xc0 };

    TEST_ASSERT_EQUAL_INT(0, ieee802154_get_frame_hdr_len(mhr));
}

static void test_ieee802154_get_frame_hdr_len_dst_2_src_2(void)
{
    const uint8_t mhr[] = { 0x00, 0x88 };

    TEST_ASSERT_EQUAL_INT(11, ieee802154_get_frame_hdr_len(mhr));
}

static void test_ieee802154_get_frame_hdr_len_dst_3_src_2(void)
{
    const uint8_t mhr[] = { 0x00, 0x8c };

    TEST_ASSERT_EQUAL_INT(17, ieee802154_get_frame_hdr_len(mhr));
}

static void test_ieee802154_get_frame_hdr_len_dst_3_src_3(void)
{
    const uint8_t mhr[] = { 0x00, 0xcc };

    TEST_ASSERT_EQUAL_INT(23, ieee802154_get_frame_hdr_len(mhr));
}

static void test_ieee802154_get_frame_hdr_len_dst_2_src_2_pan_comp(void)
{
    const uint8_t mhr[] = { 0x40, 0x88 };

    TEST_ASSERT_EQUAL_INT(9, ieee802154_get_frame_hdr_len(mhr));
}

static void test_ieee802154_get_frame_hdr_len_dst_3_src_3_pan_comp(void)
{
    const uint8_t mhr[] = { 0x40, 0xcc };

    TEST_ASSERT_EQUAL_INT(21, ieee802154_get_frame_hdr_len(mhr));
}

static void test_ieee802154_get_addr_dst_reserved(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x00, 0x04, TEST_UINT8 };

    TEST_ASSERT_EQUAL_INT(-EINVAL, ieee802154_get_addr(mhr, &src, &dst));
}

static void test_ieee802154_get_addr_src_reserved(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x00, 0x40, TEST_UINT8 };

    TEST_ASSERT_EQUAL_INT(-EINVAL, ieee802154_get_addr(mhr, &src, &dst));
}

static void test_ieee802154_get_addr_dst_0_src_0(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x00, 0x00, TEST_UINT8 };

    TEST_ASSERT_EQUAL_INT(IEEE802154_FCF_SRC_ADDR_VOID | IEEE802154_FCF_DST_ADDR_VOID,
                          ieee802154_get_addr(mhr, &src, &dst));
    TEST_ASSERT_EQUAL_INT(byteorder_htons(TEST_UINT16 + TEST_UINT8).u16, dst.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(TEST_UINT64 + TEST_UINT32).u64, dst.addr.l_addr.u64);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(TEST_UINT16).u16, src.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(TEST_UINT64).u64, src.addr.l_addr.u64);
}

static void test_ieee802154_get_addr_dst_0_src_0_pan_comp(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x40, 0x00, TEST_UINT8 };

    TEST_ASSERT_EQUAL_INT(-EINVAL, ieee802154_get_addr(mhr, &src, &dst));
}

static void test_ieee802154_get_addr_dst_2_src_0(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x00, 0x08, TEST_UINT8, 0x01, 0x23, 0x45, 0x67 };

    TEST_ASSERT_EQUAL_INT(IEEE802154_FCF_SRC_ADDR_VOID | IEEE802154_FCF_DST_ADDR_SHORT,
                          ieee802154_get_addr(mhr, &src, &dst));
    /* IEEE 802.15.4 is little-endian! */
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x2301).u16, dst.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x6745).u16, dst.addr.s_addr.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(TEST_UINT16).u16, src.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(TEST_UINT64).u64, src.addr.l_addr.u64);
}

static void test_ieee802154_get_addr_dst_2_src_0_pan_comp(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x40, 0x08, TEST_UINT8, 0x01, 0x23, 0x45, 0x67 };

    TEST_ASSERT_EQUAL_INT(IEEE802154_FCF_SRC_ADDR_VOID | IEEE802154_FCF_DST_ADDR_SHORT,
                          ieee802154_get_addr(mhr, &src, &dst));
    /* IEEE 802.15.4 is little-endian! */
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x2301).u16, dst.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x6745).u16, dst.addr.s_addr.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(TEST_UINT16).u16, src.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(TEST_UINT64).u64, src.addr.l_addr.u64);
}

static void test_ieee802154_get_addr_dst_3_src_0(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x00, 0x0c, TEST_UINT8, 0x01, 0x23, 0x45, 0x67,
                            0x89, 0xab, 0xcd, 0xef, 0x12, 0x34 };

    TEST_ASSERT_EQUAL_INT(IEEE802154_FCF_SRC_ADDR_VOID | IEEE802154_FCF_DST_ADDR_LONG,
                          ieee802154_get_addr(mhr, &src, &dst));
    /* IEEE 802.15.4 is little-endian! */
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x2301).u16, dst.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(0x3412efcdab896745).u64, dst.addr.l_addr.u64);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(TEST_UINT16).u16, src.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(TEST_UINT64).u64, src.addr.l_addr.u64);
}

static void test_ieee802154_get_addr_dst_3_src_0_pan_comp(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x40, 0x0c, TEST_UINT8, 0x01, 0x23, 0x45, 0x67,
                            0x89, 0xab, 0xcd, 0xef, 0x12, 0x34 };

    TEST_ASSERT_EQUAL_INT(IEEE802154_FCF_SRC_ADDR_VOID | IEEE802154_FCF_DST_ADDR_LONG,
                          ieee802154_get_addr(mhr, &src, &dst));
    /* IEEE 802.15.4 is little-endian! */
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x2301).u16, dst.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(0x3412efcdab896745).u64, dst.addr.l_addr.u64);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(TEST_UINT16).u16, src.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(TEST_UINT64).u64, src.addr.l_addr.u64);
}

static void test_ieee802154_get_addr_dst_0_src_2(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x00, 0x80, TEST_UINT8, 0x01, 0x23, 0x45, 0x67 };

    TEST_ASSERT_EQUAL_INT(IEEE802154_FCF_SRC_ADDR_SHORT | IEEE802154_FCF_DST_ADDR_VOID,
                          ieee802154_get_addr(mhr, &src, &dst));
    TEST_ASSERT_EQUAL_INT(byteorder_htons(TEST_UINT16 + TEST_UINT8).u16, dst.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(TEST_UINT64 + TEST_UINT32).u64, dst.addr.l_addr.u64);
    /* IEEE 802.15.4 is little-endian! */
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x2301).u16, src.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x6745).u16, src.addr.s_addr.u16);
}

static void test_ieee802154_get_addr_dst_0_src_2_pan_comp(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x40, 0x80, TEST_UINT8, 0x01, 0x23, 0x45, 0x67 };

    TEST_ASSERT_EQUAL_INT(-EINVAL, ieee802154_get_addr(mhr, &src, &dst));
}

static void test_ieee802154_get_addr_dst_2_src_2(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x00, 0x88, TEST_UINT8, 0x01, 0x23, 0x45, 0x67,
                            0x89, 0xab, 0xcd, 0xef };

    TEST_ASSERT_EQUAL_INT(IEEE802154_FCF_SRC_ADDR_SHORT | IEEE802154_FCF_DST_ADDR_SHORT,
                          ieee802154_get_addr(mhr, &src, &dst));
    /* IEEE 802.15.4 is little-endian! */
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x2301).u16, dst.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x6745).u16, dst.addr.s_addr.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0xab89).u16, src.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0xefcd).u16, src.addr.s_addr.u16);
}

static void test_ieee802154_get_addr_dst_3_src_2(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x00, 0x8c, TEST_UINT8, 0x01, 0x23, 0x45, 0x67,
                            0x89, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78,
                            0x9a, 0xbc };

    TEST_ASSERT_EQUAL_INT(IEEE802154_FCF_SRC_ADDR_SHORT | IEEE802154_FCF_DST_ADDR_LONG,
                          ieee802154_get_addr(mhr, &src, &dst));
    /* IEEE 802.15.4 is little-endian! */
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x2301).u16, dst.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(0x3412efcdab896745).u64, dst.addr.l_addr.u64);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x7856).u16, src.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0xbc9a).u16, src.addr.s_addr.u16);
}

static void test_ieee802154_get_addr_dst_0_src_3(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x00, 0xc0, TEST_UINT8, 0x01, 0x23, 0x45, 0x67,
                            0x89, 0xab, 0xcd, 0xef, 0x12, 0x34 };

    TEST_ASSERT_EQUAL_INT(IEEE802154_FCF_SRC_ADDR_LONG | IEEE802154_FCF_DST_ADDR_VOID,
                          ieee802154_get_addr(mhr, &src, &dst));
    /* IEEE 802.15.4 is little-endian! */
    TEST_ASSERT_EQUAL_INT(byteorder_htons(TEST_UINT16 + TEST_UINT8).u16, dst.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(TEST_UINT64 + TEST_UINT32).u64, dst.addr.l_addr.u64);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x2301).u16, src.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(0x3412efcdab896745).u64, src.addr.l_addr.u64);
}

static void test_ieee802154_get_addr_dst_0_src_3_pan_comp(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x41, 0xc0, TEST_UINT8, 0x01, 0x23, 0x45, 0x67,
                            0x89, 0xab, 0xcd, 0xef, 0x12, 0x34 };

    TEST_ASSERT_EQUAL_INT(-EINVAL, ieee802154_get_addr(mhr, &src, &dst));
}

static void test_ieee802154_get_addr_dst_2_src_3(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x02, 0xc8, TEST_UINT8, 0x01, 0x23, 0x45, 0x67,
                            0x89, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78,
                            0x9a, 0xbc };

    TEST_ASSERT_EQUAL_INT(IEEE802154_FCF_SRC_ADDR_LONG | IEEE802154_FCF_DST_ADDR_SHORT,
                          ieee802154_get_addr(mhr, &src, &dst));
    /* IEEE 802.15.4 is little-endian! */
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x2301).u16, dst.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x6745).u16, dst.addr.s_addr.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0xab89).u16, src.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(0xbc9a78563412efcd).u64, src.addr.l_addr.u64);
}

static void test_ieee802154_get_addr_dst_3_src_3(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x03, 0xcc, TEST_UINT8, 0x01, 0x23, 0x45, 0x67,
                            0x89, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78,
                            0x9a, 0xbc, 0xde, 0xf0, 0x10, 0x32, 0x54, 0x76 };

    TEST_ASSERT_EQUAL_INT(IEEE802154_FCF_SRC_ADDR_LONG | IEEE802154_FCF_DST_ADDR_LONG,
                          ieee802154_get_addr(mhr, &src, &dst));
    /* IEEE 802.15.4 is little-endian! */
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x2301).u16, dst.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(0x3412efcdab896745).u64, dst.addr.l_addr.u64);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x7856).u16, src.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(0x76543210f0debc9a).u64, src.addr.l_addr.u64);
}

static void test_ieee802154_get_addr_dst_3_src_3_pan_comp(void)
{
    const network_uint16_t exp_src_pan = byteorder_htons(TEST_UINT16);
    const network_uint64_t exp_src_addr = byteorder_htonll(TEST_UINT64);
    const network_uint16_t exp_dst_pan = byteorder_htons(TEST_UINT16 + TEST_UINT8);
    const network_uint64_t exp_dst_addr = byteorder_htonll(TEST_UINT64 + TEST_UINT32);
    ieee802154_addr_t src = { exp_src_pan, { exp_src_addr } };
    ieee802154_addr_t dst = { exp_dst_pan, { exp_dst_addr } };
    const uint8_t mhr[] = { 0x41, 0xcc, TEST_UINT8, 0x01, 0x23, 0x45, 0x67,
                            0x89, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78,
                            0x9a, 0xbc, 0xde, 0xf0, 0x10, 0x32 };

    TEST_ASSERT_EQUAL_INT(IEEE802154_FCF_SRC_ADDR_LONG | IEEE802154_FCF_DST_ADDR_LONG,
                          ieee802154_get_addr(mhr, &src, &dst));
    /* IEEE 802.15.4 is little-endian! */
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x2301).u16, dst.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(0x3412efcdab896745).u64, dst.addr.l_addr.u64);
    TEST_ASSERT_EQUAL_INT(byteorder_htons(0x2301).u16, src.pan.u16);
    TEST_ASSERT_EQUAL_INT(byteorder_htonll(0x3210f0debc9a7856).u64, src.addr.l_addr.u64);
}

static void test_ieee802154_get_seq(void)
{
    const uint8_t mhr[] = { 0x00, 0x00, TEST_UINT8 };

    TEST_ASSERT_EQUAL_INT(TEST_UINT8, ieee802154_get_seq(mhr));
}

static void test_ieee802154_get_iid_addr_len_0(void)
{
    const uint8_t addr[] = { 0x01, 0x23 };
    eui64_t iid;

    TEST_ASSERT_NULL(ieee802154_get_iid(&iid, addr, 0));
}

static void test_ieee802154_get_iid_addr_len_SIZE_MAX(void)
{
    const uint8_t addr[] = { 0x01, 0x23 };
    eui64_t iid;

    TEST_ASSERT_NULL(ieee802154_get_iid(&iid, addr, SIZE_MAX));
}

static void test_ieee802154_get_iid_addr_len_2(void)
{
    const uint8_t addr[] = { 0x01, 0x23 };
    const uint8_t exp[] = { 0x00, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x01, 0x23 };
    eui64_t iid;

    TEST_ASSERT_NOT_NULL(ieee802154_get_iid(&iid, addr, sizeof(addr)));
    TEST_ASSERT_EQUAL_INT(0, memcmp((const char *)exp, (char *) &iid, sizeof(iid)));
}

static void test_ieee802154_get_iid_addr_len_4(void)
{
    const uint8_t addr[] = { 0x01, 0x23, 0x45, 0x67 };
    const uint8_t exp[] = { 0x03, 0x23, 0x00, 0xff, 0xfe, 0x00, 0x45, 0x67 };
    eui64_t iid;

    TEST_ASSERT_NOT_NULL(ieee802154_get_iid(&iid, addr, sizeof(addr)));
    TEST_ASSERT_EQUAL_INT(0, memcmp((const char *)exp, (char *) &iid, sizeof(iid)));
}

static void test_ieee802154_get_iid_addr_len_8(void)
{
    const uint8_t addr[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
    const uint8_t exp[] = { 0x03, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
    eui64_t iid;

    TEST_ASSERT_NOT_NULL(ieee802154_get_iid(&iid, addr, sizeof(addr)));
    TEST_ASSERT_EQUAL_INT(0, memcmp((const char *)exp, (char *) &iid, sizeof(iid)));
}

Test *tests_ieee802154_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_ieee802154_set_frame_hdr_flags0),
        new_TestFixture(test_ieee802154_set_frame_hdr_flags0_non_beacon_non_ack),
        new_TestFixture(test_ieee802154_set_frame_hdr_reserved_dst_mode),
        new_TestFixture(test_ieee802154_set_frame_hdr_reserved_src_mode),
        new_TestFixture(test_ieee802154_set_frame_hdr_dst_2_src_0_dst_NULL),
        new_TestFixture(test_ieee802154_set_frame_hdr_dst_2_src_0_dst_NOT_NULL),
        new_TestFixture(test_ieee802154_set_frame_hdr_dst_3_src_0_dst_NULL),
        new_TestFixture(test_ieee802154_set_frame_hdr_dst_3_src_0_dst_NOT_NULL),
        new_TestFixture(test_ieee802154_set_frame_hdr_flags0_ver_1),
        new_TestFixture(test_ieee802154_set_frame_hdr_dst_0_src_2),
        new_TestFixture(test_ieee802154_set_frame_hdr_dst_0_src_3),
        new_TestFixture(test_ieee802154_set_frame_hdr_dst_2_src_2_autopan_compr),
        new_TestFixture(test_ieee802154_set_frame_hdr_dst_2_src_2),
        new_TestFixture(test_ieee802154_set_frame_hdr_dst_3_src_2),
        new_TestFixture(test_ieee802154_set_frame_hdr_dst_2_src_3),
        new_TestFixture(test_ieee802154_set_frame_hdr_dst_3_src_3),
        new_TestFixture(test_ieee802154_set_frame_hdr_dst_3_src_3_pan_comp),
        new_TestFixture(test_ieee802154_get_frame_hdr_len_src_mode),
        new_TestFixture(test_ieee802154_get_frame_hdr_len_reserved_dst_mode),
        new_TestFixture(test_ieee802154_get_frame_hdr_len_reserved_src_mode),
        new_TestFixture(test_ieee802154_get_frame_hdr_len_dst_2_src_0),
        new_TestFixture(test_ieee802154_get_frame_hdr_len_dst_3_src_0),
        new_TestFixture(test_ieee802154_get_frame_hdr_len_dst_0_src_2),
        new_TestFixture(test_ieee802154_get_frame_hdr_len_dst_0_src_3_pan_comp),
        new_TestFixture(test_ieee802154_get_frame_hdr_len_dst_0_src_2_pan_comp),
        new_TestFixture(test_ieee802154_get_frame_hdr_len_dst_0_src_3),
        new_TestFixture(test_ieee802154_get_frame_hdr_len_dst_2_src_2),
        new_TestFixture(test_ieee802154_get_frame_hdr_len_dst_3_src_2),
        new_TestFixture(test_ieee802154_get_frame_hdr_len_dst_3_src_3),
        new_TestFixture(test_ieee802154_get_frame_hdr_len_dst_2_src_2_pan_comp),
        new_TestFixture(test_ieee802154_get_frame_hdr_len_dst_3_src_3_pan_comp),
        new_TestFixture(test_ieee802154_get_addr_dst_reserved),
        new_TestFixture(test_ieee802154_get_addr_src_reserved),
        new_TestFixture(test_ieee802154_get_addr_dst_0_src_0),
        new_TestFixture(test_ieee802154_get_addr_dst_0_src_0_pan_comp),
        new_TestFixture(test_ieee802154_get_addr_dst_2_src_0),
        new_TestFixture(test_ieee802154_get_addr_dst_2_src_0_pan_comp),
        new_TestFixture(test_ieee802154_get_addr_dst_3_src_0),
        new_TestFixture(test_ieee802154_get_addr_dst_3_src_0_pan_comp),
        new_TestFixture(test_ieee802154_get_addr_dst_0_src_2),
        new_TestFixture(test_ieee802154_get_addr_dst_0_src_2_pan_comp),
        new_TestFixture(test_ieee802154_get_addr_dst_2_src_2),
        new_TestFixture(test_ieee802154_get_addr_dst_3_src_2),
        new_TestFixture(test_ieee802154_get_addr_dst_0_src_3),
        new_TestFixture(test_ieee802154_get_addr_dst_0_src_3_pan_comp),
        new_TestFixture(test_ieee802154_get_addr_dst_2_src_3),
        new_TestFixture(test_ieee802154_get_addr_dst_3_src_3),
        new_TestFixture(test_ieee802154_get_addr_dst_3_src_3_pan_comp),
        new_TestFixture(test_ieee802154_get_seq),
        new_TestFixture(test_ieee802154_get_iid_addr_len_0),
        new_TestFixture(test_ieee802154_get_iid_addr_len_SIZE_MAX),
        new_TestFixture(test_ieee802154_get_iid_addr_len_2),
        new_TestFixture(test_ieee802154_get_iid_addr_len_4),
        new_TestFixture(test_ieee802154_get_iid_addr_len_8),
    };

    EMB_UNIT_TESTCALLER(ieee802154_tests, NULL, NULL, fixtures);

    return (Test *)&ieee802154_tests;
}

void tests_ieee802154(void)
{
    TESTS_RUN(tests_ieee802154_tests());
}
/** @} */
