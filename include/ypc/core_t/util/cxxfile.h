#pragma once
#include "ypc/core_t/util/file_openmode.h"
#include "ypc/core_t/util/fpos.h"

namespace ypc {

class cxxfile {
public:
  cxxfile();

  void open(const char *filename, ypc::ios_base::openmode mode);

  void close();

  cxxfile &seekg(int64_t pos, ios_base::seekdir dir = ios_base::beg);
  int64_t tellg();

  cxxfile &seekp(int64_t pos, ios_base::seekdir dir = ios_base::beg);
  int64_t tellp();
  bool is_open() const;

  cxxfile &flush();
  cxxfile &read(uint8_t *s, size_t size);
  cxxfile &write(const uint8_t *s, size_t size);

  bool good() const;
  bool eof() const;
  bool fail() const;
  bool bad() const;

protected:
  uint32_t m_stream_id;
};
} // namespace ypc
