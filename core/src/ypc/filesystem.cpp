#include "ypc/filesystem.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>

namespace ypc {
std::string home_directory() { return std::string(getenv("HOME")); }
std::string current_directory() {
  auto t = boost::filesystem::current_path();
  return t.generic_string();
}

std::string complete_path(const std::string &ph, const std::string &base) {
  auto t = boost::filesystem::complete(boost::filesystem::path(ph),
                                       boost::filesystem::path(base));
  return t.generic_string();
}

std::string dirname(const std::string &ph) {
  boost::filesystem::path p(ph);
  return p.parent_path().generic_string();
}

std::string join_path(const std::string &ph, const std::string &sub) {
  boost::filesystem::path p1(ph);
  return (p1 / sub).generic_string();
}

bool is_portable_name(const std::string &name) {
  return boost::filesystem::portable_name(name);
}
bool is_dir_exists(const std::string &path) {
  boost::filesystem::path p(path);
  return boost::filesystem::is_directory(p) && boost::filesystem::exists(p);
}
bool is_file_exists(const std::string &path) {
  boost::filesystem::path p(path);
  return boost::filesystem::is_regular_file(p) && boost::filesystem::exists(p);
}

void copyDirectoryRecursively(const boost::filesystem::path &sourceDir,
                              const boost::filesystem::path &destinationDir) {
  namespace fs = boost::filesystem;
  if (!fs::exists(sourceDir) || !fs::is_directory(sourceDir)) {
    throw std::runtime_error("Source directory " + sourceDir.string() +
                             " does not exist or is not a directory");
  }
  if (fs::exists(destinationDir)) {
    throw std::runtime_error("Destination directory " +
                             destinationDir.string() + " already exists");
  }
  if (!fs::create_directory(destinationDir)) {
    throw std::runtime_error("Cannot create destination directory " +
                             destinationDir.string());
  }

  for (const auto &dirEnt : fs::recursive_directory_iterator{sourceDir}) {
    const auto &path = dirEnt.path();
    auto relativePathStr = path.string();
    boost::replace_first(relativePathStr, sourceDir.string(), "");
    fs::copy(path, destinationDir / relativePathStr);
  }
}
void copy_directory(const std::string &from, const std::string &to) {
  copyDirectoryRecursively(boost::filesystem::path(from),
                           boost::filesystem::path(to));
}
} // namespace ypc
