#pragma once
#include "ypc/corecommon/exceptions.h"
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#ifdef HPDA_DEBUG
#include <glog/logging.h>
#endif
#include "ypc/core_t/util/cxxfile.h"
#include "ypc/core_t/util/file_openmode.h"
#include "ypc/core_t/util/fpos.h"
#include "ypc/corecommon/blockfile/blockfile_v1.h"
#include "ypc/corecommon/blockfile/traits.h"

namespace ypc {

template <> struct file_traits<cxxfile> {
  constexpr static auto in = ypc::ios_base::in;
  constexpr static auto binary = ypc::ios_base::binary;
  constexpr static auto out = ypc::ios_base::out;
  constexpr static auto trunc = ypc::ios_base::trunc;
  constexpr static auto ate = ypc::ios_base::ate;
  constexpr static auto app = ypc::ios_base::app;
  constexpr static auto cur = ypc::ios_base::cur;
  constexpr static auto beg = ypc::ios_base::beg;
  constexpr static auto end = ypc::ios_base::end;
};

template <uint64_t MagicNumber_t, uint64_t BlockNumLimit_t,
          uint64_t BlockSizeLimit_t>
using blockfile = blockfile_v1<ypc::cxxfile, MagicNumber_t, BlockNumLimit_t,
                               BlockSizeLimit_t>;

} // namespace ypc
