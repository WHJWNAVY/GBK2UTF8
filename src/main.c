/*
 * Copyright (c) 2020 Louis Suen
 * Licensed under the MIT License. See the LICENSE file for the full text.
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "gbk2uni.h"

int32_t read_file_to_buff(const char *file, uint8_t **fbuff, uint32_t *pflen) {
    int ret = 0;
    FILE *fp = NULL;
    uint8_t *pbuff = NULL;
    uint32_t fsize = 0;
    uint32_t rsize = 0;

    if ((file == NULL) || (fbuff == NULL) || (pflen == NULL)) {
        LOGE("Invalid file name!");
        ret = -1;
        goto err;
    }

    fp = fopen(file, "rb");
    if (fp == NULL) {
        LOGE("Failed to open file [%s]", file);
        ret = -1;
        goto err;
    }

    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    if (fsize <= 0) {
        LOGE("Invalid file size [%u]!", fsize);
        ret = fsize;
        goto err;
    }
    fseek(fp, 0, SEEK_SET);

    pbuff = malloc(fsize);
    if (pbuff == NULL) {
        LOGE("Failed to malloc size [%u]!", fsize);
        ret = fsize;
        goto err;
    }
    memset(pbuff, 0, fsize);

    while ((rsize = fread(pbuff, 1, fsize, fp)) <= 0) {
        if (errno == EINTR || errno == EAGAIN) {
            errno = 0;
            continue;
        }
        break;
    }

    if (rsize != fsize) {
        LOGE("Failed to read file to buffer, size [%u:%u]!", rsize, fsize);
        ret = rsize;
        goto err;
    }

    LOGD("Read file [%s] to addr [%p] size [%u]", file, pbuff, fsize);

    *fbuff = pbuff;
    *pflen = fsize;
    ret = 0;
err:
    if (fp != NULL) {
        fclose(fp);
    }
    if ((ret != 0) && (pbuff != NULL)) {
        free(pbuff);
    }
    return ret;
}

int32_t write_buff_to_file(uint8_t *fbuff, uint32_t flen, const char *file) {
    int ret = 0;
    uint32_t wlen = 0;
    FILE *fp = NULL;

    if ((file == NULL) || (fbuff == NULL) || (flen <= 0)) {
        LOGE("Invalid file name!");
        ret = -1;
        goto err;
    }

    fp = fopen(file, "wb");
    if (fp == NULL) {
        LOGE("Failed to open file [%s]", file);
        ret = -1;
        goto err;
    }

    wlen = fwrite(fbuff, 1, flen, fp);
    if (wlen != flen) {
        LOGE("Failed to write buffer to file, size [%u:%u]!", wlen, flen);
        ret = -wlen;
        goto err;
    }

    LOGD("Write buffer [%p] size [%u] to file [%s]", fbuff, flen, file);
    ret = 0;
err:
    if (fp != NULL) {
        fclose(fp);
    }
    return ret;
}

static void usage(const char *exe_name) {
    printf("Usage: %s <INPUT_FILE> [OUTPUT_FILE]\n", exe_name);
}
int main(int argc, char *argv[]) {
    int32_t ret = 0;
    char *in_file = NULL;
    char *out_file = NULL;
    uint8_t *in_buff = NULL;
    uint8_t *out_buff = NULL;
    uint32_t in_len = 0;
    uint32_t out_len = 0;

    if ((argc < 2) || (NULL == argv[1]) || (strlen(argv[1]) <= 0)) {
        LOGE("Invalid input filename!");
        ret = 1;
        goto __oops;
    }
    in_file = argv[1];
    LOGD("Input file[%s]", in_file);

    if ((argc > 2) && (NULL != argv[2]) && (strlen(argv[2]) > 0)) {
        out_file = argv[2];
    }

    LOGD("Output file[%s]", ((out_file != NULL) ? out_file : "stdout"));

    ret = read_file_to_buff(in_file, &in_buff, &in_len);
    if (ret != 0) {
        LOGE("Failed to read file [%s]!", in_file);
        goto __oops;
    }
    LOGD("Input buff[%p], len[%u]", in_buff, in_len);
    if (is_valid_gbkns(in_buff, in_len)) {
        LOGD("Is valid gbk string!");
        out_buff = gbk2utf8(in_buff, in_len);
    } else if (is_valid_utf8ns(in_buff, in_len)) {
        LOGD("Is valid utf8 string!");
        out_buff = strdup(in_buff);
    } else if (is_printns(in_buff, in_len)) {
        LOGD("Is ascii string!");
        out_buff = strdup(in_buff);
    } else {
        LOGE("Unknow encode!");
        ret = -1;
        goto __oops;
    }

    if (out_buff == NULL) {
        LOGE("Failed to decode gbk string!");
        ret = -1;
        goto __oops;
    }
    out_len = strlen(out_buff);
    LOGD("output buff[%p], len[%u]", out_buff, out_len);

    if (out_file != NULL) {
        LOGD("Write buff to file [%s]!", out_file);
        ret = write_buff_to_file(out_buff, out_len, out_file);
        if (ret != 0) {
            LOGE("Failed to write buff to file [%s]!", out_file);
            goto __oops;
        }
    } else {
        LOGD("Write buff to stdout:");
        printf("%s\n", out_buff);
    }

    ret = 0;
__oops:
    if (ret > 0) {
        usage(argv[0]);
    }

    if (out_buff != NULL) {
        free(out_buff);
    }

    if (in_buff != NULL) {
        free(in_buff);
    }

    return ret;
}
