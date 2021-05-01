#pragma once

#include "common/byte.h"

#include <cstring>
#include <memory>
#include <string>

namespace stbox {

typedef uint8_t byte_t;
using bytes = ::ypc::utc::bytes<uint8_t, ::ypc::utc::byte_encode::raw_bytes>;
using hex_bytes = bytes::hex_bytes_t;
using base58_bytes = bytes::base58_bytes_t;

} // namespace stbox

