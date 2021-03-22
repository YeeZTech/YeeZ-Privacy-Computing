#pragma once
#include <hpda/common/common.h>

namespace hpda {
struct lazy_eval_stream_policy {};
struct prefetch_stream_policy {};
struct fetch_all_stream_policy {};

using default_stream_policy = lazy_eval_stream_policy;
} // namespace hpda

