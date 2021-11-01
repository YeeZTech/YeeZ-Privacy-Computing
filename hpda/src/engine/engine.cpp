#include <hpda/engine/engine.h>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>

namespace hpda {

void engine::add_functor(functor *f) {
  m_functors.push_back(f);
}

void engine::run() {
  build_graph();
  auto outputs = find_outputs();
  if (outputs.size() != 1) {
    throw std::runtime_error("only support 1 output this version");
  }
  std::queue<functor *> to_process;
  std::stack<functor *> processing;
  for (auto output : outputs) {
    to_process.push(output);
  }

  int i = 0;
  while (!to_process.empty()) {
    do {
      auto f = to_process.front();
      to_process.pop();
      if (f->predecessors().empty()) {
        // it's an extractor
        bool v = f->process();
        if (v) {
          f->done_value();
        } else {
          m_reach_ends.insert(f);
        }
      } else {
        processing.push(f);
      }
      for (auto input : f->predecessors()) {
        if (!input->has_value()) {
          to_process.push(input);
        }
      }

      // if (!to_process.empty() && !processing.empty()) {
      // if (!contain_same_successor(to_process.front(), f, processing.top())) {
      // break;
      //}
      //}
    } while (!to_process.empty());

    while (!processing.empty()) {
      //<< std::endl;
      auto f = processing.top();

      if (!functor_has_input(f)) {
        break;
      }

      bool v = f->process();
      if (v) {
        f->done_value();
      } else {
        f->reset_done_value();
      }
      // for (auto input : f->predecessors()) {
      // input->reset_done_value();
      //}
      if (f->has_value()) {
        processing.pop();
      } else {
        bool end = true;
        for (auto input : f->predecessors()) {
          if (m_reach_ends.find(input) == m_reach_ends.end() &&
              !(input->has_value())) {
            to_process.push(input);
            end = false;
          }
        }
        if (end) {
          m_reach_ends.insert(f);
          processing.pop();
        }
      }
    }
    if (to_process.empty()) {
      for (auto output : outputs) {
        if (m_reach_ends.find(output) == m_reach_ends.end()) {
          to_process.push(output);
        }
      }
    }
  }
}

// We assume there is no circle, it's a DAG
void engine::build_graph() {
  for (auto f : m_functors) {
    if (m_predecessors.find(f) == m_predecessors.end()) {
      m_predecessors.insert(std::make_pair(f, std::unordered_set<functor *>()));
    }
    if (m_successors.find(f) == m_successors.end()) {
      m_successors.insert(std::make_pair(f, std::unordered_set<functor *>()));
    }
    for (auto input : f->predecessors()) {
      if (m_successors.find(input) == m_successors.end()) {
        m_successors.insert(
            std::make_pair(input, std::unordered_set<functor *>()));
      }
      m_successors[input].insert(f);
      m_predecessors[f].insert(input);
    }
  }
}
std::vector<functor *> engine::find_outputs() const {
  std::vector<functor *> ret;
  for (auto it : m_successors) {
    if (it.second.empty()) {
      ret.push_back(it.first);
    }
  }
  return ret;
}

bool engine::contain_same_successor(functor *f1, functor *f2,
                                    functor *succ) const {
  auto s1 = m_successors.find(f1)->second;
  auto s2 = m_successors.find(f2)->second;
  if (s1.find(succ) != s1.end() && s2.find(succ) != s2.end()) {
    return true;
  }
  return false;
}

bool engine::functor_has_input(functor *f) const {
  const std::vector<functor *> &inputs = f->predecessors();
  bool ret = true;
  for (auto input : inputs) {
    if (m_reach_ends.find(input) != m_reach_ends.end()) {
      continue;
    }
    ret = ret && input->has_value();
  }
  return ret;
}

bool engine::is_output(functor *f) const {
  return m_successors.find(f)->second.empty();
}
} // namespace hpda
