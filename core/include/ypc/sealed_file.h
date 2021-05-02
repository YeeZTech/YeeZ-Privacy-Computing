#pragma once
#include "common/limits.h"
#include "ypc/blockfile.h"
#include "ypc/byte.h"
#include "ypc/memref.h"
#include "ypc/ntobject_file.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace ypc {

namespace internal {
class sealed_file_base {
public:
  typedef blockfile<0x4788d13e7fefe21f, 1024 * 1024,
                    256 * ::ypc::utc::max_item_size>
      blockfile_t;

  sealed_file_base(const std::string &file_path, bool read);

  virtual ~sealed_file_base();
  virtual void write_item(const bytes &data);

  virtual void reset_read() = 0;

  virtual bool next_item(memref &s) = 0;

protected:
  blockfile_t m_file;
};
} // namespace internal

class simple_sealed_file : public internal::sealed_file_base {
public:
  simple_sealed_file(const std::string &file_path, bool read);
  virtual void reset_read();
  virtual bool next_item(memref &s);
};

// We use cache to improve read performance
class sealed_file_with_cache_opt : public internal::sealed_file_base {
public:
  sealed_file_with_cache_opt(const std::string &file_path, bool read);

  virtual ~sealed_file_with_cache_opt();

  virtual void reset_read();

  virtual bool next_item(memref &s);

protected:
  std::unique_ptr<std::thread> m_io_thread;
  std::queue<memref> m_cached;
  bool m_reach_end;
  std::mutex m_mutex;
  std::condition_variable m_cond_full;
  std::condition_variable m_cond_empty;
  uint32_t m_max_queue_size;
  std::atomic_bool m_to_close;
};
} // namespace ypc

define_nt(sfm_path, std::string);
// define_nt(sfm_hash, std::string);
// define_nt(sfm_path, ypc::bytes);
define_nt(sfm_hash, ypc::bytes);
define_nt(sfm_index, uint16_t);
define_nt(sfm_num, uint16_t);
typedef ::ff::net::ntpackage<1, sfm_path, sfm_index, sfm_hash> sfm_item_t;
define_nt(sfm_items, std::vector<sfm_item_t>);

typedef ::ff::net::ntpackage<2, sfm_items, sfm_num, sfm_hash> sfm_t;
