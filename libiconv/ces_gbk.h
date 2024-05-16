/*
 * Copyright (C) 1999-2001, 2005, 2016 Free Software Foundation, Inc.
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

/*
 * GBK
 */

int32_t ces_gbk_mbtowc(conv_t conv, ucs4_t *pwc, const uint8_t *s, size_t n) {
    uint8_t c = *s;

    /* Code set 0 (ASCII or GB 1988-89) */
    if (c < 0x80)
        return ascii_mbtowc(conv, pwc, s, n);
    /* Code set 1 (GBK) */
    if (c >= 0x81 && c < 0xff) {
        if (n < 2)
            return RET_TOOFEW(0);
        return gbk_mbtowc(conv, pwc, s, 2);
    }
    return RET_ILSEQ;
}

int32_t ces_gbk_wctomb(conv_t conv, uint8_t *r, ucs4_t wc, size_t n) {
    uint8_t buf[2];
    int32_t ret;

    /* Code set 0 (ASCII or GB 1988-89) */
    ret = ascii_wctomb(conv, r, wc, n);
    if (ret != RET_ILUNI)
        return ret;

    /* Code set 1 (GBK) */
    ret = gbk_wctomb(conv, buf, wc, 2);
    if (ret != RET_ILUNI) {
        if (ret != 2)
            abort();
        if (n < 2)
            return RET_TOOSMALL;
        r[0] = buf[0];
        r[1] = buf[1];
        return 2;
    }

    return RET_ILUNI;
}
