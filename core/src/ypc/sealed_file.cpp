#include "ypc/sealed_file.h"

namespace ypc {
namespace internal {
sealed_file_base::sealed_file_base(const std::string &file_path, bool read) {
  if (read) {
    m_file.open_for_read(file_path.c_str());
    m_file.reset_read_item();
  } else {
    m_file.open_for_write(file_path.c_str());
  }
}

sealed_file_base::~sealed_file_base() {}
void sealed_file_base::write_item(const bytes &data) {
  m_file.append_item((const char *)data.data(), data.size());
}
} // namespace internal

simple_sealed_file::simple_sealed_file(const std::string &file_path, bool read)
    : sealed_file_base(file_path, read) {}

void simple_sealed_file::reset_read() { m_file.reset_read_item(); }
bool simple_sealed_file::next_item(memref &s) { return m_file.next_item(s); }

sealed_file_with_cache_opt::sealed_file_with_cache_opt(
    const std::string &file_path, bool read)
    : sealed_file_base(file_path, read) {
  m_reach_end = false;
  m_to_close = false;
  m_max_queue_size = 16;

  m_io_thread.reset(new std::thread([&]() {
    while (!m_reach_end) {
      std::unique_lock<std::mutex> l(m_mutex);
      while (!m_to_close && !m_reach_end &&
             m_cached.size() >= m_max_queue_size) {
        m_cond_full.wait(l);
      }
      if (m_to_close) {
        break;
      }

      uint32_t s = m_cached.size();
      l.unlock();

      std::vector<memref> ms;
      while (!m_reach_end && s <= m_max_queue_size) {
        memref tmr;
        m_reach_end = !(m_file.next_item(tmr));
        if (!m_reach_end) {
          ms.push_back(tmr);
          s++;
        }
      }

      l.lock();
      bool empty = m_cached.empty();
      for (auto mr : ms) {
        m_cached.push(mr);
      }
      if (empty) {
        m_cond_empty.notify_one();
      }
      l.unlock();
    }
  }));
}

sealed_file_with_cache_opt::~sealed_file_with_cache_opt() {
  m_to_close = true;
  m_cond_empty.notify_all();
  m_cond_full.notify_all();
  m_io_thread->join();
}

void sealed_file_with_cache_opt::reset_read() {
  std::unique_lock<std::mutex> l(m_mutex);
  bool full = (m_cached.size() >= m_max_queue_size);
  while (!m_cached.empty()) {
    m_cached.pop();
  }

  if (full) {
    m_cond_full.notify_all();
  }
}
bool sealed_file_with_cache_opt::next_item(memref &s) {
  std::unique_lock<std::mutex> l(m_mutex);
  while (!m_to_close && !m_reach_end && m_cached.size() == 0) {
    m_cond_empty.wait(l);
  }

  if (m_to_close) {
    return false;
  }
  if (m_reach_end && m_cached.size() == 0) {
    return false;
  }
  bool full = (m_cached.size() >= m_max_queue_size);
  s = m_cached.front();
  m_cached.pop();
  if (full) {
    m_cond_full.notify_one();
  }
  return true;
}
} // namespace ypc
