#pragma once
#include "ypc/core/exceptions.h"

namespace ypc {
class invalid_blockfile : public std::exception {
public:
  virtual const char *what() { return "wrong magic number"; }
};

class blockfile_interface {
public:

  virtual void reset_read_item() = 0;
  virtual int next_item(char *buf, size_t in_size, size_t &out_size) = 0;
  virtual uint64_t item_number() = 0;
  virtual void close() = 0;
  virtual int append_item(const char *data, size_t len) = 0;
  virtual void open_for_read(const char *file_path) = 0;
  virtual void open_for_write(const char *file_path) = 0;
};
} // namespace ypc
