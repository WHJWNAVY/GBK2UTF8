#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "converters.h"

#define LOG(LEVEL, FMT, ...)                                                     \
    do {                                                                         \
        fprintf(stderr, "(%s:%d) " FMT "\n", __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define PRINT_DEBUG(FMT, ...) LOG(LOG_DEBUG, FMT, ##__VA_ARGS__)
#define PRINT_ERROR(FMT, ...) LOG(LOG_ERR, FMT, ##__VA_ARGS__)

#ifndef __is_print
// #define __is_print(ch) ((uint32_t)((ch) - ' ') < 127u - ' ')
#define __is_print(ch) isprint(ch)
#endif

#ifdef __dump_ret
#undef __dump_ret
#endif
#define __dump_ret(_ptr, _l, _n, _m) \
    ({                               \
        _l = strlen(_ptr);           \
        _n += _l;                    \
        if (_n >= _m) {              \
            return;                  \
        }                            \
        _ptr += _l;                  \
    })

void print_hex(const uint8_t *buf, uint32_t size) {
    int32_t i = 0, j = 0;
    uint32_t number = 16; // The number of outputs per line

    if ((buf == NULL) || (size == 0)) {
        return;
    }

    for (i = 0; i < size; i += number) {
        printf("%08X: ", i);

        for (j = 0; j < number; j++) {
            if (j % 8 == 0) {
                printf(" ");
            }
            if (i + j < size) {
                printf("%02X ", buf[i + j]);
            } else {
                printf("   ");
            }
        }
        printf(" |  ");

        for (j = 0; j < number; j++) {
            if (i + j < size) {
                printf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
            }
        }
        printf("\n");
    }
}

void dump_hex(char *out, uint32_t len, const uint8_t *buf, uint32_t size) {
    int32_t i = 0, j = 0;
    uint32_t number = 16; // The number of outputs per line
    uint32_t l = 0, n = 0;
    char *ptr = out;

    if ((out == NULL) || (buf == NULL) || (size == 0) || (len == 0)) {
        return;
    }

    memset(out, 0, len);
    for (i = 0; i < size; i += number) {
        sprintf(ptr, "%08X: ", i);
        __dump_ret(ptr, l, n, len);
        for (j = 0; j < number; j++) {
            if (j % 8 == 0) {
                sprintf(ptr, " ");
                __dump_ret(ptr, l, n, len);
            }
            if (i + j < size) {
                sprintf(ptr, "%02X ", buf[i + j]);
                __dump_ret(ptr, l, n, len);
            } else {
                sprintf(ptr, "   ");
                __dump_ret(ptr, l, n, len);
            }
        }
        sprintf(ptr, " |  ");
        __dump_ret(ptr, l, n, len);

        for (j = 0; j < number; j++) {
            if (i + j < size) {
                sprintf(ptr, "%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
                __dump_ret(ptr, l, n, len);
            }
        }
        sprintf(ptr, "\n");
        __dump_ret(ptr, l, n, len);
    }
    *ptr = '\0';
}

size_t unicode_loop_convert(conv_t cd, const uint8_t **inbuf, size_t *inbytesleft, uint8_t **outbuf,
                            size_t *outbytesleft) {
    size_t result = 0;
    ucs4_t wc = 0;
    int32_t incount = 0, outcount = 0;
    state_t last_istate = 0;

    const uint8_t *inptr = *inbuf;
    int32_t inleft = *inbytesleft;
    uint8_t *outptr = *outbuf;
    int32_t outleft = *outbytesleft;

    while (inleft > 0) {
        PRINT_DEBUG("inptr=%p, inleft=%d, outptr=%p, outleft=%d", inptr, inleft, outptr, outleft);
        last_istate = cd->istate;
        incount = cd->ifuncs(cd, &wc, inptr, inleft);
        PRINT_DEBUG("incount=%d, wc=0x%x, inptr=%p, inleft=%u, outptr=%p, outleft=%u, result=%u", incount, wc, inptr,
                    inleft, outptr, outleft, result);
        if (incount < 0) {
            if ((uint32_t)(-1 - incount) % 2 == (uint32_t)(-1 - RET_ILSEQ) % 2) {
                /* Case 1: invalid input, possibly after a shift sequence */
                incount = DECODE_SHIFT_ILSEQ(incount);
                PRINT_DEBUG("Case 1: invalid input, possibly after a shift sequence, incount=%d", incount);

                inptr += incount;
                inleft -= incount;
                errno = EILSEQ;
                result = -1;
                break;
            }
            if (incount == RET_TOOFEW(0)) {
                /* Case 2: not enough bytes available to detect anything */
                PRINT_DEBUG("Case 2: not enough bytes available to detect anything");
                errno = EINVAL;
                result = -1;
                break;
            }
            /* Case 3: k bytes read, but only a shift sequence */
            incount = DECODE_TOOFEW(incount);
            PRINT_DEBUG("Case 3: k bytes read, but only a shift sequence, incount=%d", incount);
        } else {
            /* Case 4: k bytes read, making up a wide character */
            PRINT_DEBUG("Case 4: k bytes read, making up a wide character");
            if (outleft == 0) {
                PRINT_DEBUG("outleft=%u", outleft);
                cd->istate = last_istate;
                errno = E2BIG;
                result = -1;
                break;
            }
            outcount = cd->ofuncs(cd, outptr, wc, outleft);
            PRINT_DEBUG("incount=%d, wc=0x%x, inptr=%p, inleft=%u, outptr=%p, outleft=%u, result=%u", incount, wc,
                        inptr, inleft, outptr, outleft, result);
            if (outcount != RET_ILUNI) {
                PRINT_DEBUG("goto outcount_ok");
                goto outcount_ok;
            }
            /* Handle Unicode tag characters (range U+E0000..U+E007F). */
            if ((wc >> 7) == (0xe0000 >> 7)) {
                PRINT_DEBUG("goto outcount_zero");
                goto outcount_zero;
            }
            /* Try transliteration. */
            result++;

            outcount = cd->ofuncs(cd, outptr, 0xFFFD, outleft);
            PRINT_DEBUG("incount=%d, wc=0x%x, inptr=%p, inleft=%u, outptr=%p, outleft=%u, result=%u", incount, wc,
                        inptr, inleft, outptr, outleft, result);
            if (outcount != RET_ILUNI) {
                PRINT_DEBUG("goto outcount_ok");
                goto outcount_ok;
            }
            cd->istate = last_istate;
            errno = EILSEQ;
            result = -1;
            break;
        outcount_ok:
            if (outcount < 0) {
                PRINT_DEBUG("outcount_ok:outcount=%u", outcount);
                cd->istate = last_istate;
                errno = E2BIG;
                result = -1;
                break;
            }

            if (!(outcount <= outleft)) {
                PRINT_DEBUG("outcount_ok:outcount=%u, outleft=%u", outcount, outleft);
                result = -1;
                return result;
            }
            outptr += outcount;
            outleft -= outcount;
            PRINT_DEBUG("outptr=%p, outleft=%u", outptr, outleft);
        }
    outcount_zero:
        if (!(incount <= inleft)) {
            PRINT_DEBUG("outcount_zero:incount=%d, inleft=%u", incount, inleft);
            result = -1;
            return result;
        }
        inptr += incount;
        inleft -= incount;
        PRINT_DEBUG("inptr=%p, inleft=%u", inptr, inleft);
    }
    *inbuf = (const char *)inptr;
    *inbytesleft = inleft;
    *outbuf = (char *)outptr;
    *outbytesleft = outleft;
    return result;
}

int32_t main(int32_t argc, char *argv[]) {
    iconv_t conv = {
        .ifuncs = gbk_mbtowc,
        .ofuncs = utf8_wctomb,
    };

    uint8_t gbk[] = {0xCE, 0xD2, 0xCA, 0xC7, 0xD6, 0xD0, 0xB9, 0xFA, 0xC8, 0xCB, 0x00, 0x00};
    // uint8_t gbk[] = {0xE6, 0x88, 0x91, 0xE6, 0x98, 0xAF, 0xE4, 0xB8, 0xAD, 0xE5, 0x9B, 0xBD, 0xE4, 0xBA, 0xBA, 0x00, 0x00, 0x00};
    uint8_t utf[100] = {0};
    uint8_t *pgbk = gbk;
    uint8_t *putf = utf;
    uint32_t ilen = strlen(gbk);
    // uint32_t ilen = sizeof(gbk);
    uint32_t olen = sizeof(utf);
    uint32_t osz = unicode_loop_convert(&conv, &pgbk, &ilen, &putf, &olen);
    PRINT_DEBUG("osz=%d, ilen=%d, olen=%d\n", osz, ilen, olen);
    print_hex(utf, sizeof(utf));

    return 0;
}