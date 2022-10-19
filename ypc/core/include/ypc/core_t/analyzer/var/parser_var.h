#pragma once
#include "hpda/engine/engine.h"

namespace ypc {
namespace internal {

template <typename ParserT> class parser_var {
protected:
  std::shared_ptr<ParserT> m_parser;
  std::shared_ptr<hpda::engine> m_engine;
};
} // namespace internal
} // namespace ypc
