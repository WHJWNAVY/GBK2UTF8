/*
 * Copyright (c) 2020 Louis Suen
 * Licensed under the MIT License. See the LICENSE file for the full text.
 */

#ifndef __TAB_GBK2UNI_H__
#define __TAB_GBK2UNI_H__

// https://github.com/yeahlouis/GBK2UTF8

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool is_valid_gbk(const uint8_t *data, size_t len);
bool is_valid_utf8(const uint8_t *data, size_t len);
int32_t uni2utf8(uint16_t ns, uint8_t buf[4]);
char *gbk2utf8(const uint8_t *data, size_t len);
bool is_printns(const char *str, size_t len);
bool is_prints(const char *str);
bool is_valid_gbkns(const char *str, size_t len);
bool is_valid_gbks(const char *str);
bool is_valid_utf8ns(const char *str, size_t len);
bool is_valid_utf8s(const char *str);

#endif
