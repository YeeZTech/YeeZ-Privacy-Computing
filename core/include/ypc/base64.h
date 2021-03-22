#pragma once
#include <string>

namespace ypc {
bool decode_base64(const std::string &val, std::string &output);
std::string encode_base64(const std::string &val);
} // namespace ypc

