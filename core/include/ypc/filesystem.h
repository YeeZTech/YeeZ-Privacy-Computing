#pragma once
#include <string>

namespace ypc {
std::string home_directory();
std::string current_directory();
std::string complete_path(const std::string &ph,
                          const std::string &base = current_directory());
bool is_portable_name(const std::string &name);

bool is_dir_exists(const std::string &path);
bool is_file_exists(const std::string &path);

void copy_directory(const std::string &from, const std::string &to);

} // namespace ypc
