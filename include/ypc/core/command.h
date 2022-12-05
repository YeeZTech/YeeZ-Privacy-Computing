#pragma once
#include "ypc/core/singleton.h"
#include <vector>

namespace ypc {

using command_type_t = uint64_t;

struct base_command {
  base_command(const base_command &) = delete;
  base_command &operator=(const base_command &) = delete;
  base_command(base_command &&) = delete;
  base_command &operator=(base_command &&) = delete;
  virtual ~base_command() = default;
}; // end class base_command

struct exit_command : public base_command {
  constexpr static command_type_t command_type = 1;
}; // end class exit_command

class command_queue : public singleton<command_queue> {
public:
  inline command_queue()
      : singleton<command_queue>(), m_handlers(), m_mutex(){};

  template <typename CT> void send_command(const std::shared_ptr<CT> &cmd) {
    std::lock_guard<std::mutex> _l(m_mutex);
    for (auto it : m_handlers) {
      if (CT::command_type == it.m_command) {
        it.m_handler(cmd);
      }
    }
  }

  template <typename CT>
  void
  listen_command(void *instance,
                 const std::function<void(const std::shared_ptr<CT> &)> &func) {
    std::lock_guard<std::mutex> _l(m_mutex);
    m_handlers.push_back(
        command_entity_t(CT::command_type, instance,
                         [func](const std::shared_ptr<base_command> &cmd) {
                           func(std::static_pointer_cast<CT>(cmd));
                         }));
  }

  template <typename CT> void unlisten_command(void *instance) {
    //! Notice that we may have multiple handlers for the same instance,
    // and we need remove them all.
    std::lock_guard<std::mutex> _l(m_mutex);
    for (auto it = m_handlers.rbegin(); it != m_handlers.rend(); ++it) {
      if (it->m_command == CT::command_type && it->m_instance == instance) {
        m_handlers.erase(it.base() - 1);
      }
    }
  }
  inline void unlisten_command(void *instance) {
    //! Notice that we may have multiple handlers for the same instance,
    // and we need remove them all.
    std::lock_guard<std::mutex> _l(m_mutex);
    for (auto it = m_handlers.rbegin(); it != m_handlers.rend(); ++it) {
      if (it->m_instance == instance) {
        m_handlers.erase(it.base() - 1);
      }
    }
  }

protected:
  struct command_entity {
    inline command_entity(
        command_type_t cmd, void *instance,
        const std::function<void(const std::shared_ptr<base_command> &)> &f)
        : m_command(cmd), m_instance(instance), m_handler(f) {}
    command_type_t m_command;
    void *m_instance;
    std::function<void(const std::shared_ptr<base_command> &)> m_handler;
  };
  using command_entity_t = command_entity;
  using handlers_t = std::vector<command_entity_t>;
  handlers_t m_handlers;
  std::mutex m_mutex;
}; // end class command_queue
} // namespace ypc
