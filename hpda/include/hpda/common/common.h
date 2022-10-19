#pragma once

#include <cstdlib>
#include <exception>
#include <ff/util/ntobject.h>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#ifdef HPDA_DEBUG
#include <glog/logging.h>
#endif

#ifdef YPC_SGX
#include "ypc/stbox/tsgx/log.h"
#endif

namespace hpda {
template <typename... ARGS> using ntobject = ::ff::util::ntobject<ARGS...>;

}
