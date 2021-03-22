#include "ypc/base64.h"
#include "ypc/byte.h"
#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>

namespace ypc {

bool decode_base64(const std::string &val, std::string &output) {
  using namespace boost::archive::iterators;
  using It =
      transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
  output = boost::algorithm::trim_right_copy_if(
      std::string(It(std::begin(val)), It(std::end(val))),
      [](char c) { return c == '\0'; });
  return true;
}

std::string encode_base64(const std::string &val) {
  using namespace boost::archive::iterators;
  using It =
      base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
  auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
  return tmp.append((3 - val.size() % 3) % 3, '=');
}

} // namespace ypc
