#pragma once
#include <vector>

namespace hpda {
class engine;
class functor {
public:
  functor();
  virtual ~functor();
  virtual bool process() = 0;
  inline void reset_done_value() { m_has_value = false; }
  inline void done_value() { m_has_value = true; }
  inline bool has_value() const { return m_has_value; }
  inline engine *get_engine() const { return m_engine; }

  void set_engine(engine *e);

  inline void add_predecessor(functor *pred) { m_predecessors.push_back(pred); }

  inline const std::vector<functor *> predecessors() const {
    return m_predecessors;
  }

protected:
  bool m_has_value;
  bool m_has_more_data;
  std::vector<functor *> m_predecessors;
  engine *m_engine;
};
} // namespace ypc
