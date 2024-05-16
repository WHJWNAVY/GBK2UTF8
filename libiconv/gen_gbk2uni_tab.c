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

#define _s(x) ((uint16_t)(x))
#define merge_b16b(h, l) ((_s(l) << 8) | _s(h))
#define merge_b16l(h, l) ((_s(h) << 8) | _s(l))

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define swap_b16(x) (((_s(x) & 0x00ff) << 8) | ((_s(x) & 0xff00) >> 8))
#define merge_b16 merge_b16l
#else
#define swap_b16(_x) (_x)
#define merge_b16 merge_b16b
#endif

int32_t main(int32_t argc, char *argv[]) {
    uint16_t gbkc = 0;
    uint16_t gbkl = 0;
    uint16_t gbkh = 0;
    ucs4_t wc = 0;
    int32_t ret = 0;

#if 0 // test big-little endian
    gbkl = 0xD2;
    gbkh = 0x04;
    gbkc = merge_b16l(gbkh, gbkl);
    // OUTPUT: 0x04D2, 0x04D2, 0x04D2, 0x04D2
    printf("0x%02X%02X, 0x%04X, 0x%04X, 0x%04X\n", gbkh, gbkl, gbkc, __BYTE_ORDER__, __ORDER_LITTLE_ENDIAN__);
    gbkc = merge_b16b(gbkh, gbkl);
    // OUTPUT: 0x04D2, 0xD204, 0x04D2, 0x04D2
    printf("0x%02X%02X, 0x%04X, 0x%04X, 0x%04X\n", gbkh, gbkl, gbkc, __BYTE_ORDER__, __ORDER_LITTLE_ENDIAN__);

    gbkl = 0xCE;
    gbkh = 0xD2;
    gbkc = merge_b16l(gbkh, gbkl);
    ret = gbk_mbtowc(NULL, &wc, &gbkc, sizeof(gbkc));
    // OUTPUT: 0xD2CE, 0xD2CE, 0x6211
    printf("0x%02X%02X, 0x%04X, 0x%04X\n", gbkh, gbkl, gbkc, wc);
    gbkc = merge_b16b(gbkh, gbkl);
    ret = gbk_mbtowc(NULL, &wc, &gbkc, sizeof(gbkc));
    // OUTPUT: 0xD2CE, 0xCED2, 0x6905
    printf("0x%02X%02X, 0x%04X, 0x%04X\n", gbkh, gbkl, gbkc, wc);

    uint8_t *ptr = (uint8_t *)&gbkc;
    ptr[0] = 0xCE;
    ptr[1] = 0xD2;
    ret = gbk_mbtowc(NULL, &wc, &gbkc, sizeof(gbkc));
    // OUTPUT: 0xCED2, 0xD2CE, 0x6211
    printf("0x%02X%02X, 0x%04X, 0x%04X\n", ptr[0], ptr[1], gbkc, wc);
#elif 0
    for (gbkh = 0; gbkh <= 0xff; gbkh++) {
        for (gbkl = 0; gbkl <= 0xff; gbkl++) {
            gbkc = merge_b16(gbkh, gbkl);
            ret = gbk_mbtowc(NULL, &wc, &gbkc, sizeof(gbkc));
            if (ret > 0) {
                printf("0x%02X%02X, 0x%04X, 0x%04X\n", gbkh, gbkl, gbkc, wc);
            } else {
                printf("0x%02X%02X, 0x%04X, %d\n", gbkh, gbkl, gbkc, ret);
            }
        }
    }
#elif 0
    uint32_t line = 0;
    printf("#ifndef __GBK2UNICODE_TABLE_H__\n");
    printf("#define __GBK2UNICODE_TABLE_H__\n\n");
    printf("static const uint16_t GBK2UNICODE_TABLE[] = {\n\t");
    for (gbkh = 0; gbkh <= 0xff; gbkh++) {
        for (gbkl = 0; gbkl <= 0xff; gbkl++) {
            gbkc = merge_b16(gbkh, gbkl);
            ret = gbk_mbtowc(NULL, &wc, &gbkc, sizeof(gbkc));
            if (ret <= 0) {
                wc = 0;
            }

            printf("0x%04X, ", wc);
            if (((++line) % 16) == 0) {
                printf("\n\t");
            }
        }
    }
    printf("};\n\n");
    printf("#define GBK2UNICODE_TABLE_SIZE (sizeof(GBK2UNICODE_TABLE) / sizeof(uint16_t))\n\n");
    printf("#endif\n\n");
#elif 1
    uint32_t start = 0;
    uint32_t line = 0;
    printf("#ifndef __GBK2UNICODE_TABLE_H__\n");
    printf("#define __GBK2UNICODE_TABLE_H__\n\n");
    printf("static const uint16_t GBK2UNICODE_TABLE[] = {\n\t");
    for (gbkh = 0; gbkh <= 0xff; gbkh++) {
        for (gbkl = 0; gbkl <= 0xff; gbkl++) {
            gbkc = merge_b16(gbkh, gbkl);
            ret = gbk_mbtowc(NULL, &wc, &gbkc, sizeof(gbkc));
            if (ret > 0) {
                if (start == 0) {
                    start = gbkc;
                }
            } else {
                wc = 0;
            }
            if (start) {
                printf("0x%04X, ", wc);
                if (((++line) % 16) == 0) {
                    printf("\n\t");
                }
            }
        }
    }
    printf("};\n\n");
    printf("#define GBK2UNICODE_TABLE_SIZE (sizeof(GBK2UNICODE_TABLE) / sizeof(uint16_t))\n");
    printf("#define GBK2UNICODE_TABLE_START 0x%04X\n\n", start);
    printf("#endif\n\n");
#else
    for (gbkc = 0; gbkc <= 0xffff; gbkc++) {
        ret = gbk_mbtowc(NULL, &wc, &gbkc, sizeof(gbkc));
        if (ret > 0) {
            printf("0x%04X, 0x%04X\n", gbkc, wc);
        } else {
            printf("0x%04X, %d\n", gbkc, ret);
        }
    }
#endif
    return 0;
}