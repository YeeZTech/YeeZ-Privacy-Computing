#include "parser_registry.h"

#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iomanip>
#include <openssl/sha.h>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>

namespace fid {
namespace {

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;

class scoped_descriptor final {
public:
  explicit scoped_descriptor(int descriptor) : descriptor_(descriptor) {}
  scoped_descriptor(const scoped_descriptor &) = delete;
  scoped_descriptor &operator=(const scoped_descriptor &) = delete;
  scoped_descriptor(scoped_descriptor &&) = delete;
  scoped_descriptor &operator=(scoped_descriptor &&) = delete;
  ~scoped_descriptor() {
    if (descriptor_ >= 0) {
      (void)::close(descriptor_);
    }
  }
  int get() const noexcept { return descriptor_; }

private:
  int descriptor_;
};

std::string read_registry_fragment(int descriptor, const std::string &path) {
  constexpr size_t kMaximumFragmentSize = 1024u * 1024u;
  std::string result;
  char buffer[16 * 1024];
  while (true) {
    const ssize_t count = ::read(descriptor, buffer, sizeof(buffer));
    if (count < 0 && errno == EINTR) {
      continue;
    }
    if (count < 0) {
      throw std::runtime_error("cannot read registry fragment '" + path +
                               "': " + std::strerror(errno));
    }
    if (count == 0) {
      break;
    }
    if (result.size() + static_cast<size_t>(count) > kMaximumFragmentSize) {
      throw std::runtime_error(path + ": registry fragment exceeds 1 MiB");
    }
    result.append(buffer, static_cast<size_t>(count));
  }
  return result;
}

std::string required_string(const pt::ptree &tree, const char *name,
                            const std::string &fragment) {
  const auto value = tree.get_optional<std::string>(name);
  if (!value || value->empty()) {
    throw std::runtime_error(fragment + ": missing non-empty field '" + name +
                             "'");
  }
  return *value;
}

std::string first_string(const pt::ptree &tree,
                         const std::vector<std::string> &names) {
  for (const auto &name : names) {
    const auto value = tree.get_optional<std::string>(name);
    if (value && !value->empty()) {
      return *value;
    }
  }
  return std::string();
}
void require_unique_object_keys(const pt::ptree &tree,
                                const std::string &fragment) {
  std::map<std::string, size_t> counts;
  for (const auto &field : tree) {
    if (field.first.empty()) {
      throw std::runtime_error(fragment +
                               ": registry root must be a JSON object");
    }
    if (++counts[field.first] != 1) {
      throw std::runtime_error(fragment + ": duplicate field '" + field.first +
                               "'");
    }
  }
}

std::string resolve_library(const std::string &fragment,
                            const std::string &relative) {
  const fs::path path(relative);
  if (path.is_absolute()) {
    throw std::runtime_error(fragment +
                             ": module library path must be relative");
  }
  return fs::absolute(fs::path(fragment).parent_path() / path).string();
}

std::string bytes_hex(const ypc::bytes &value) {
  std::ostringstream stream;
  stream << value;
  return normalize_hex(stream.str());
}

bool is_hex_string(const std::string &value, size_t expected_size) {
  return value.size() == expected_size &&
         std::all_of(value.begin(), value.end(), [](char character) {
           return std::isxdigit(static_cast<unsigned char>(character)) != 0;
         });
}

bool has_untrusted_write_permissions(
    const struct stat &status, bool allow_effective_group_write = false) {
  if ((status.st_mode & S_IWOTH) != 0) {
    return true;
  }
  return (status.st_mode & S_IWGRP) != 0 &&
         (!allow_effective_group_write || status.st_gid != ::getegid());
}

void require_trusted_directory(const std::string &path) {
  struct stat status = {};
  if (::lstat(path.c_str(), &status) != 0 || S_ISLNK(status.st_mode) ||
      !S_ISDIR(status.st_mode) || has_untrusted_write_permissions(status) ||
      (status.st_uid != 0 && status.st_uid != ::geteuid())) {
    throw std::runtime_error(
        "parser registry directory has an untrusted owner or permissions: " +
        path);
  }
}

} // namespace

std::string normalize_hex(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](char character) {
    return static_cast<char>(
        std::tolower(static_cast<unsigned char>(character)));
  });
  if (value.compare(0, 7, "sha256:") == 0) {
    value.erase(0, 7);
  }
  if (value.compare(0, 2, "0x") == 0) {
    value.erase(0, 2);
  }

  std::string result;
  result.reserve(value.size());
  for (char character : value) {
    if (std::isspace(static_cast<unsigned char>(character)) == 0 &&
        character != ':') {
      result.push_back(character);
    }
  }
  return result;
}

bool valid_module_id(const std::string &value) {
  if (value.empty() || value.size() > 128 ||
      value.find(".so") != std::string::npos ||
      value.find('/') != std::string::npos ||
      value.find(static_cast<char>(92)) != std::string::npos) {
    return false;
  }
  for (char character : value) {
    const auto byte = static_cast<unsigned char>(character);
    if (!((std::isalnum(byte) != 0) || character == '.' || character == '_' ||
          character == '-')) {
      return false;
    }
  }
  return true;
}

void parser_registry::require_trusted_fd(int descriptor,
                                        const char *description,
                                        bool allow_effective_group_write) {
  struct stat status = {};
  if (::fstat(descriptor, &status) != 0) {
    throw std::runtime_error(std::string("cannot fstat ") + description +
                             ": " + std::strerror(errno));
  }
  if (!S_ISREG(status.st_mode) ||
      has_untrusted_write_permissions(status, allow_effective_group_write) ||
      (status.st_uid != 0 && status.st_uid != ::geteuid())) {
    throw std::runtime_error(std::string(description) +
                             " has an untrusted owner or permissions");
  }
}

void parser_registry::require_trusted_file(
    const std::string &path, const char *description,
    bool allow_effective_group_write) {
  struct stat status = {};
  if (::lstat(path.c_str(), &status) != 0) {
    throw std::runtime_error(std::string("cannot stat ") + description + " '" +
                             path + "': " + std::strerror(errno));
  }
  if (S_ISLNK(status.st_mode) || !S_ISREG(status.st_mode)) {
    throw std::runtime_error(std::string(description) +
                             " must be a regular non-symlink file: " + path);
  }
  if (has_untrusted_write_permissions(status, allow_effective_group_write) ||
      (status.st_uid != 0 && status.st_uid != ::geteuid())) {
    throw std::runtime_error(std::string(description) +
                             " has an untrusted owner or permissions: " + path);
  }
}

std::string parser_registry::sha256_fd(int descriptor) {
  SHA256_CTX context;
  if (SHA256_Init(&context) != 1) {
    throw std::runtime_error("cannot initialize SHA-256");
  }

  char buffer[64 * 1024];
  off_t offset = 0;
  while (true) {
    const ssize_t count = ::pread(descriptor, buffer, sizeof(buffer), offset);
    if (count < 0 && errno == EINTR) {
      continue;
    }
    if (count < 0) {
      throw std::runtime_error(std::string("cannot read file for SHA-256: ") +
                               std::strerror(errno));
    }
    if (count == 0) {
      break;
    }
    if (SHA256_Update(&context, buffer, static_cast<size_t>(count)) != 1) {
      throw std::runtime_error("cannot update SHA-256");
    }
    offset += count;
  }

  unsigned char digest[SHA256_DIGEST_LENGTH];
  if (SHA256_Final(digest, &context) != 1) {
    throw std::runtime_error("cannot finalize SHA-256");
  }
  std::ostringstream result;
  result << std::hex << std::setfill('0');
  for (unsigned char byte : digest) {
    result << std::setw(2) << static_cast<unsigned int>(byte);
  }
  return result.str();
}

std::string parser_registry::sha256_file(const std::string &path) {
  const int descriptor =
      ::open(path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
  if (descriptor < 0) {
    throw std::runtime_error("cannot open file for SHA-256 '" + path +
                             "': " + std::strerror(errno));
  }
  scoped_descriptor file(descriptor);
  return sha256_fd(file.get());
}

void parser_registry::load(const std::vector<std::string> &directories) {
  if (directories.empty()) {
    throw std::runtime_error("no parser registry directory configured");
  }

  for (const auto &directory : directories) {
    const fs::path path(directory);
    if (!fs::exists(path) || !fs::is_directory(path)) {
      throw std::runtime_error("parser registry directory does not exist: " +
                               directory);
    }
    require_trusted_directory(directory);

    std::vector<std::string> fragments;
    for (fs::directory_iterator iterator(path), end; iterator != end;
         ++iterator) {
      if (fs::is_regular_file(iterator->status()) &&
          iterator->path().extension() == ".json") {
        fragments.push_back(iterator->path().string());
      }
    }
    std::sort(fragments.begin(), fragments.end());
    for (const auto &fragment : fragments) {
      load_fragment(fragment);
    }
  }

  for (const auto &entry : m_enclaves_by_sha256) {
    const auto module = m_modules.find(entry.second.module_id);
    if (module == m_modules.end()) {
      throw std::runtime_error(
          entry.second.fragment_path +
          ": enclave references an unknown parser module '" +
          entry.second.module_id + "'");
    }
    if (module->second.edl_fingerprint != entry.second.edl_fingerprint) {
      throw std::runtime_error(
          entry.second.fragment_path +
          ": enclave EDL fingerprint conflicts with its parser module");
    }
  }
}

void parser_registry::load_fragment(const std::string &path) {
  const int descriptor =
      ::open(path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
  if (descriptor < 0) {
    throw std::runtime_error("cannot open registry fragment '" + path +
                             "': " + std::strerror(errno));
  }
  scoped_descriptor fragment(descriptor);
  require_trusted_fd(fragment.get(), "registry fragment");
  pt::ptree tree;
  try {
    const std::string contents = read_registry_fragment(fragment.get(), path);
    std::istringstream stream(contents);
    pt::read_json(stream, tree);
  } catch (const std::exception &error) {
    throw std::runtime_error(path +
                             ": malformed registry fragment: " + error.what());
  }
  require_unique_object_keys(tree, path);
  if (tree.get<uint32_t>("schema_version", 0) != 1) {
    throw std::runtime_error(path + ": unsupported registry schema_version");
  }

  const std::string type = required_string(tree, "type", path);
  if (type == "parser_module") {
    parser_module_record record;
    record.fragment_path = path;
    record.module_id = required_string(tree, "module_id", path);
    record.abi_major = tree.get<uint32_t>("abi_major");
    record.abi_minor = tree.get<uint32_t>("abi_minor", 0);
    record.edl_fingerprint =
        normalize_hex(required_string(tree, "edl_fingerprint", path));
    const std::string library =
        first_string(tree, {"relative_library_path", "library"});
    record.library_sha256 =
        normalize_hex(required_string(tree, "library_sha256", path));

    if (!valid_module_id(record.module_id)) {
      throw std::runtime_error(path + ": invalid module_id");
    }
    if (!is_hex_string(record.edl_fingerprint, SHA256_DIGEST_LENGTH * 2)) {
      throw std::runtime_error(path + ": invalid EDL fingerprint");
    }
    if (library.empty()) {
      throw std::runtime_error(path + ": missing relative_library_path");
    }
    if (!is_hex_string(record.library_sha256, SHA256_DIGEST_LENGTH * 2)) {
      throw std::runtime_error(path + ": invalid library SHA-256");
    }
    record.library_path = resolve_library(path, library);

    if (!m_modules.emplace(record.module_id, record).second) {
      throw std::runtime_error(path + ": duplicate module_id '" +
                               record.module_id + "'");
    }
    return;
  }

  if (type == "parser_enclave") {
    parser_enclave_record record;
    record.fragment_path = path;
    record.module_id = required_string(tree, "module_id", path);
    record.edl_fingerprint =
        normalize_hex(required_string(tree, "edl_fingerprint", path));
    record.enclave_sha256 =
        normalize_hex(required_string(tree, "signed_file_sha256", path));
    record.mrenclave = normalize_hex(required_string(tree, "mrenclave", path));

    if (!valid_module_id(record.module_id)) {
      throw std::runtime_error(path + ": invalid module_id");
    }
    if (!is_hex_string(record.edl_fingerprint, SHA256_DIGEST_LENGTH * 2)) {
      throw std::runtime_error(path + ": invalid EDL fingerprint");
    }
    if (!is_hex_string(record.enclave_sha256, SHA256_DIGEST_LENGTH * 2) ||
        !is_hex_string(record.mrenclave, 64)) {
      throw std::runtime_error(path + ": invalid enclave SHA-256 or MRENCLAVE");
    }
    if (!m_enclaves_by_sha256.emplace(record.enclave_sha256, record).second) {
      throw std::runtime_error(path + ": duplicate signed enclave SHA-256");
    }
    return;
  }

  throw std::runtime_error(path + ": unknown fragment type '" + type + "'");
}

parser_registry_selection
parser_registry::select(const std::string &module_id,
                        const std::string &enclave_path,
                        const ypc::bytes &expected_mrenclave) const {
  if (!valid_module_id(module_id)) {
    throw std::runtime_error("invalid parser module ID: " + module_id);
  }
  const auto module = m_modules.find(module_id);
  if (module == m_modules.end()) {
    throw std::runtime_error("unknown parser module ID: " + module_id);
  }

  require_trusted_file(module->second.library_path, "parser module");
  if (sha256_file(module->second.library_path) !=
      module->second.library_sha256) {
    throw std::runtime_error("parser module SHA-256 mismatch: " +
                             module->second.library_path);
  }

  // Published parser enclaves are measured SGX artifacts rather than native
  // host plugins. Preserve existing 0664 deployments only when the group is
  // the analyzer's effective trusted group; SHA-256 and MRENCLAVE checks below
  // still bind the exact signed bytes before any parser ECALL is accepted.
  require_trusted_file(enclave_path, "signed enclave", true);
  const std::string canonical_enclave = fs::canonical(enclave_path).string();
  require_trusted_file(canonical_enclave, "signed enclave", true);
  const std::string enclave_sha256 = sha256_file(canonical_enclave);
  const auto enclave = m_enclaves_by_sha256.find(enclave_sha256);
  if (enclave == m_enclaves_by_sha256.end()) {
    throw std::runtime_error("signed enclave is not registered (SHA-256 " +
                             enclave_sha256 + ")");
  }
  if (enclave->second.module_id != module_id) {
    throw std::runtime_error("signed enclave is registered for module '" +
                             enclave->second.module_id + "', not '" +
                             module_id + "'");
  }
  if (enclave->second.edl_fingerprint != module->second.edl_fingerprint) {
    throw std::runtime_error(
        "EDL fingerprint mismatch between module and signed enclave");
  }

  const std::string expected = bytes_hex(expected_mrenclave);
  if (!is_hex_string(expected, 64) || expected != enclave->second.mrenclave) {
    throw std::runtime_error(
        "task parser_enclave_hash does not match registered MRENCLAVE");
  }

  parser_registry_selection selection;
  selection.module = module->second;
  selection.enclave = enclave->second;
  selection.enclave_path = canonical_enclave;
  return selection;
}

} // namespace fid
