#include "iris_reader.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

void *create_item_reader(const char *file_path, int len) {
  std::ifstream *s = new std::ifstream(file_path);

  return s;
}

int reset_for_read(void *handle) {
  if (!handle) {
    return -1;
  }
  std::ifstream *is = (std::ifstream *)handle;
  is->clear();
  return 0;
}

int read_item_data(void *handle, char *buf, int *len) {
  if (!handle) {
    return -1;
  }
  if (!buf) {
    return -1;
  }

  std::string s;
  std::ifstream *is = (std::ifstream *)handle;
  std::getline(*is, s);
  memcpy(buf, s.c_str(), s.size());
  *len = s.size();

  return 0;
}

int close_item_reader(void *handle) {
  std::ifstream *is = (std::ifstream *)handle;
  is->close();
  return 0;
}

uint64_t get_item_number(void *handle) {
  std::ifstream *is = (std::ifstream *)handle;
  is->seekg(0, is->beg);
  uint64_t n = 0;
  std::string s = "s";
  while (s.size() > 0 && !is->eof()) {
    s.clear();
    std::getline(*is, s);
    if (s.size() > 0) {
      n++;
    }
  }
  // must clear ifstream before seekg
  is->clear();
  is->seekg(0, is->beg);
  return n;
}
