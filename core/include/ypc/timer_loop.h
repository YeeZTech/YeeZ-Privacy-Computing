#pragma once
#include "ypc/command.h"
#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>

namespace ypc {

class timer_loop {
public:
  inline timer_loop(boost::asio::io_service *service)
      : m_service(service), m_exit_flag(false) {
    command_queue::instance().listen_command<exit_command>(
        this,
        [this](const std::shared_ptr<exit_command> &) { m_exit_flag = true; });
  }

  template <typename Func>
  void register_timer_and_callback(long seconds, Func &&f) {

    auto timer = std::make_unique<boost::asio::deadline_timer>(
        *m_service, boost::posix_time::seconds(seconds));

    m_timers.push_back(std::move(timer));
    std::unique_ptr<boost::asio::deadline_timer> &t = m_timers.back();
    auto pt = t.get();

    pt->async_wait([this, pt, seconds, f](const boost::system::error_code &ec) {
      timer_callback(ec, seconds, pt, f);
    });
  }

protected:
  void timer_callback(const boost::system::error_code &ec, long seconds,
                      boost::asio::deadline_timer *timer,
                      std::function<void()> func);

protected:
  boost::asio::io_service *m_service;
  std::atomic_bool m_exit_flag;
  std::vector<std::unique_ptr<boost::asio::deadline_timer>> m_timers;
};
} // namespace ypc
