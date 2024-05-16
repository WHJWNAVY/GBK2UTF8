/*
 * Copyright (C) 1999-2002, 2004-2011, 2016, 2022 Free Software Foundation, Inc.
 * This file is part of the GNU LIBICONV Library.
 *
 * The GNU LIBICONV Library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * The GNU LIBICONV Library is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the GNU LIBICONV Library; see the file COPYING.LIB.
 * If not, see <https://www.gnu.org/licenses/>.
 */

/* This file defines all the converters. */

/* Our own notion of wide character, as UCS-4, according to ISO-10646-1. */
typedef uint32_t ucs4_t;

/* State used by a conversion. 0 denotes the initial state. */
typedef uint32_t state_t;

typedef struct conv_struct *conv_t;

/* Return code if invalid input after a shift sequence of n bytes was read.
   (xxx_mbtowc) */
#define RET_SHIFT_ILSEQ(n) (-1 - 2 * (n))
/* Return code if invalid. (xxx_mbtowc) */
#define RET_ILSEQ RET_SHIFT_ILSEQ(0)
/* Return code if only a shift sequence of n bytes was read. (xxx_mbtowc) */
#define RET_TOOFEW(n) (-2 - 2 * (n))
/* Retrieve the n from the encoded RET_... value. */
#define DECODE_SHIFT_ILSEQ(r) ((uint32_t)(RET_SHIFT_ILSEQ(0) - (r)) / 2)
#define DECODE_TOOFEW(r) ((uint32_t)(RET_TOOFEW(0) - (r)) / 2)
/* Maximum value of n that may be used as argument to RET_SHIFT_ILSEQ or RET_TOOFEW. */
#define RET_COUNT_MAX ((INT_MAX / 2) - 1)

/* Return code if invalid. (xxx_wctomb) */
#define RET_ILUNI -1
/* Return code if output buffer is too small. (xxx_wctomb, xxx_reset) */
#define RET_TOOSMALL -2

typedef int32_t (*mbtowc_funcs)(conv_t conv, ucs4_t *pwc, uint8_t const *s, size_t n);
typedef int32_t (*wctomb_funcs)(conv_t conv, uint8_t *r, ucs4_t wc, size_t n);

typedef struct conv_struct {
    state_t istate;
    state_t ostate;
    mbtowc_funcs ifuncs;
    wctomb_funcs ofuncs;
} iconv_t;

/*
 * Include all the converters.
 */

#include "ascii.h"

/* General multi-byte encodings */
#include "utf16.h"
#include "utf16be.h"
#include "utf16le.h"
#include "utf8.h"

/* CJK character sets [CCS = coded character set] [CJKV.INF chapter 3] */

typedef struct {
    uint16_t indx; /* index into big table */
    uint16_t used; /* bitmask of used entries */
} Summary16;

#include "gb2312.h"
#include "gbk.h"

/* CJK encodings [CES = character encoding scheme] [CJKV.INF chapter 4] */
#include "ces_gbk.h"
#include "cp936.h"
#include "gb18030.h"
