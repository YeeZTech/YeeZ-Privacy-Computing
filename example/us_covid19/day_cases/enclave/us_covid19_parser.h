#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

int parse_item_data(const uint8_t *data, int len, void *item);

#ifdef __cplusplus
}
#endif
