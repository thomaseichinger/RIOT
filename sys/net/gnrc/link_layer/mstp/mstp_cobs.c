
#include <stdio.h>
#include <stdint.h>

#define MSTP_COBS_RESERVED_VALUE    (0x00)
/*
 * StuffData byte stuffs "length" bytes of
 * data at the location pointed to by "ptr",
 * writing the output to the location pointed
 * to by "dst".
 */
void mstp_cobs_stuff_data(const uint8_t *src, size_t length, uint8_t *dst)
{
    const uint8_t *end = src + length;
    uint8_t *code_ptr = dst++;
    uint8_t code = 0x01;

    while (src < end) {
        if (*src == MSTP_COBS_RESERVED_VALUE) {
            *code_ptr = code;
            code_ptr = dst++;
            code = 0x01;
        }
        else {
            *dst++ = *src;
            if (++code == 0xff) {
                *code_ptr = code;
                code_ptr = dst++;
                code = 0x01;
            }
        }
        src++;
    }
    *code_ptr = code;
    code_ptr = dst++;
    code = 0x01;
}

/*
 * Defensive UnStuffData, which prevents poorly
 * conditioned data at *ptr from over-running
 * the available buffer at *dst.
 */
void mstp_cobs_unstuff_data(const uint8_t *src, size_t length, uint8_t *dst)
{
    const uint8_t *end = src + length;
    while (src < end) {
        int code = *src++;
        for(int i = 1; src<end && i<code; i++) {
            *dst++ = *src++;
        }
        if (code < 0xff) {
            *dst++ = MSTP_COBS_RESERVED_VALUE;
        }
    }
}