#include "person_reader.h"
#include "../common.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

void *create_item_reader(const char *file_path, int len) {

  file_t *f = new file_t();
  f->open_for_read(file_path);
  f->reset_read_item();
  return f;
}

int reset_for_read(void *handle) {
  if (!handle) {
    return -1;
  }
  file_t *f = (file_t *)handle;
  f->reset_read_item();
  return 0;
}

std::unique_ptr<char[]> g_buf;
size_t g_buf_size;

int read_item_data(void *handle, char *buf, int *len) {
  if (!handle) {
    return -1;
  }
  if (!buf) {
    return -1;
  }
  file_t *f = (file_t *)handle;

  if (g_buf_size == 0) {
    g_buf_size = file_t::BlockSizeLimit;
    g_buf.reset(new char[g_buf_size]);
  }
  size_t s;
  bool t = f->next_item(buf, *len, s) == file_t::succ;
  if (t) {
    *len = s;
    return 0;
  } else {
    *len = 0;
    return -1;
  }

  return 0;
}

int close_item_reader(void *handle) {
  file_t *f = (file_t *)handle;
  f->close();
  return 0;
}

uint64_t get_item_number(void *handle) {
  file_t *f = (file_t *)handle;
  return f->item_number();
}
