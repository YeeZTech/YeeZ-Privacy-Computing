#pragma once
#include "ypc/core/exceptions.h"
#include <cassert>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#ifdef HPDA_DEBUG
#include <glog/logging.h>
#endif
#include "ypc/corecommon/blockfile/blockfile_v1.h"
#include "ypc/corecommon/blockfile/traits.h"

namespace ypc {

template <> struct file_traits<std::fstream> {
  constexpr static auto in = std::ios::in;
  constexpr static auto binary = std::ios::binary;
  constexpr static auto out = std::ios::out;
  constexpr static auto trunc = std::ios::trunc;
  constexpr static auto ate = std::ios::ate;
  constexpr static auto app = std::ios::app;
  constexpr static auto cur = std::ios::cur;
  constexpr static auto beg = std::ios::beg;
  constexpr static auto end = std::ios::end;
};

template <uint64_t MagicNumber_t, uint64_t BlockNumLimit_t,
          uint64_t BlockSizeLimit_t>
using blockfile = blockfile_v1<std::fstream, MagicNumber_t, BlockNumLimit_t,
                               BlockSizeLimit_t>;

} // namespace ypc
