#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

void *create_item_reader(const char *file_path, int len);

int reset_for_read(void *handle);
int read_item_data(void *handle, char *buf, int *len);
int close_item_reader(void *handle);
uint64_t get_item_number(void *handle);

#ifdef __cplusplus
}
#endif
