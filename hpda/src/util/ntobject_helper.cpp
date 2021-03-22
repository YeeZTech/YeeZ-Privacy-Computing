#include <hpda/util/ntobject_helper.h>
#include <sstream>

namespace hpda {
namespace util {
std::string convert_data_format_to_ntobject_type_file(
    const boost::property_tree::ptree &tree) {
  std::stringstream ss;
  return tree.get<int>("start");
}
} // namespace util
} // namespace hpda
