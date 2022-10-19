#pragma once
#include <hpda/common/common.h>
#include <boost/property_tree/ptree.hpp>

namespace hpda {
namespace util {
std::string convert_data_format_to_ntobject_type_file(
    const boost::property_tree::ptree &tree);
}
} // namespace hpda
