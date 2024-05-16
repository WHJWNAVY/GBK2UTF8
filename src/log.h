#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#define LOG(LEVEL, FMT, ...)                                                     \
    do {                                                                         \
        fprintf(stderr, "(%s:%d) " FMT "\n", __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define LOGD(FMT, ...) LOG(LOG_DEBUG, FMT, ##__VA_ARGS__)
#define LOGW(FMT, ...) LOG(LOG_WARNING, FMT, ##__VA_ARGS__)
#define LOGE(FMT, ...) LOG(LOG_ERR, FMT, ##__VA_ARGS__)

#endif
