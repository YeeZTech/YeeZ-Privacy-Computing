#include "ypc/timer_loop.h"
#include <glog/logging.h>

namespace ypc {

void timer_loop::timer_callback(const boost::system::error_code &ec,
                                long seconds,
                                boost::asio::deadline_timer *timer,
                                std::function<void()> func) {
  if (m_exit_flag)
    return;
  if (ec) {
    LOG(ERROR) << ec;
    return;
  }
  func();
  timer->expires_at(timer->expires_at() + boost::posix_time::seconds(seconds));
  timer->async_wait(
      [this, timer, seconds, func](const boost::system::error_code &ec) {
        timer_callback(ec, seconds, timer, func);
      });
}
} // namespace ypc
