#include "ypc/core/timer_loop.h"
#include <glog/logging.h>

namespace ypc {

void timer_loop::timer_callback(const boost::system::error_code &ec,
                                uint64_t seconds,
                                boost::asio::deadline_timer *timer,
                                const std::function<void()> &func) {
  if (m_exit_flag) {
    return;
  }
  if (ec) { // NOLINT
    LOG(ERROR) << ec;
    return;
  }
  func();
  timer->expires_at(
      timer->expires_at() +
      boost::posix_time::seconds( // NOLINT: Allow
                                  // fuchsia-default-arguments-calls
          seconds));
  timer->async_wait(
      [this, timer, seconds, func](const boost::system::error_code &ec) {
        timer_callback(ec, seconds, timer, func);
      });
}
} // namespace ypc
