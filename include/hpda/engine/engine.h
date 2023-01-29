/**@file engine.h
 * @brief A class to represent the computing task
 * @details Developers can modify algorithms using templates to satisfy certain
 * business scenarios
 * @author YeeZTech
 * @date 2022-10-19
 * @version 2.0
 */
#pragma once
#include <hpda/engine/functor.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hpda {
class engine {
public:
  //! Notice that one functor may add to engine multiple times, and it won't
  //! affect.
  void add_functor(functor *f);

  void remove_functor(functor * f);

  void run();

protected:

  void build_graph();

  std::vector<functor *> find_outputs() const;

  bool contain_same_successor(functor *f1, functor *f2, functor *succ) const;

  bool functor_has_input(functor *f) const;

  bool is_output(functor *f) const;

protected:
  std::vector<functor *> m_functors;

  std::unordered_map<functor *, std::unordered_set<functor *>> m_successors;
  std::unordered_map<functor *, std::unordered_set<functor *>> m_predecessors;
  std::unordered_set<functor *> m_reach_ends;
};
} // namespace ypc
