#include "iodef.h"
#include "parser_registry.h"
#include "plugin_parser.h"
#include "ypc/core/ntjson.h"
#include "ypc/core/status.h"
#include "ypc/core/version.h"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

#ifndef FID_DEFAULT_PARSER_REGISTRY_DIR
#define FID_DEFAULT_PARSER_REGISTRY_DIR                                        \
  "/usr/local/share/fidelius/parser-registry.d"
#endif

namespace {

// Parser module used when the task does not name one of its own.
const char *const kDefaultModule = "fidelius.eparser.v1";

boost::program_options::options_description options() {
  namespace po = boost::program_options;
  po::options_description result("Fidelius Analyzer options");
  result.add_options()("help", "show this help")(
      "version", "show Fidelius version")("input", po::value<std::string>(),
                                          "input parameters JSON file")(
      "output", po::value<std::string>(),
      "output result JSON file")("gen-example-input", po::value<std::string>(),
                                 "generate example input parameters JSON file")(
      "parser-registry-dir", po::value<std::vector<std::string>>()->composing(),
      "append a trusted parser registry fragment directory");
  return result;
}

class temporary_output final {
public:
  temporary_output(int descriptor, std::string path)
      : descriptor_(descriptor), path_(std::move(path)) {}

  temporary_output(const temporary_output &) = delete;
  temporary_output &operator=(const temporary_output &) = delete;
  temporary_output(temporary_output &&) = delete;
  temporary_output &operator=(temporary_output &&) = delete;

  ~temporary_output() {
    const int saved_errno = errno;
    if (descriptor_ >= 0) {
      ::close(descriptor_);
    }
    if (!path_.empty()) {
      ::unlink(path_.c_str());
    }
    errno = saved_errno;
  }

  int descriptor() const noexcept { return descriptor_; }
  const std::string &path() const noexcept { return path_; }

  int close_descriptor() noexcept {
    const int descriptor = descriptor_;
    descriptor_ = -1;
    return ::close(descriptor);
  }

  void published() noexcept { path_.clear(); }

private:
  int descriptor_;
  std::string path_;
};

void write_output_atomically(const std::string &path,
                             const std::string &result) {
  const std::string pattern = path + ".tmp.XXXXXX";
  std::vector<char> writable_pattern(pattern.begin(), pattern.end());
  writable_pattern.push_back('\0');

  const int descriptor = ::mkstemp(writable_pattern.data());
  if (descriptor < 0) {
    const int error = errno;
    throw std::runtime_error("cannot create temporary output for '" + path +
                             "': " + std::strerror(error));
  }
  temporary_output temporary(descriptor, writable_pattern.data());

  const int descriptor_flags = ::fcntl(descriptor, F_GETFD);
  if (descriptor_flags < 0 ||
      ::fcntl(descriptor, F_SETFD, descriptor_flags | FD_CLOEXEC) != 0) {
    const int error = errno;
    throw std::runtime_error("cannot secure temporary output for '" + path +
                             "': " + std::strerror(error));
  }

  size_t offset = 0;
  constexpr size_t kMaximumWrite = 1024u * 1024u;
  while (offset < result.size()) {
    const size_t remaining = result.size() - offset;
    const size_t requested =
        remaining < kMaximumWrite ? remaining : kMaximumWrite;
    const ssize_t written =
        ::write(descriptor, result.data() + offset, requested);
    if (written < 0 && errno == EINTR) {
      continue;
    }
    if (written <= 0) {
      const int error = written == 0 ? EIO : errno;
      throw std::runtime_error("cannot write temporary output for '" + path +
                               "': " + std::strerror(error));
    }
    offset += static_cast<size_t>(written);
  }

  if (::fsync(descriptor) != 0) {
    const int error = errno;
    throw std::runtime_error("cannot sync temporary output for '" + path +
                             "': " + std::strerror(error));
  }
  if (temporary.close_descriptor() != 0) {
    const int error = errno;
    throw std::runtime_error("cannot close temporary output for '" + path +
                             "': " + std::strerror(error));
  }
  if (::rename(temporary.path().c_str(), path.c_str()) != 0) {
    const int error = errno;
    throw std::runtime_error("cannot publish output '" + path +
                             "': " + std::strerror(error));
  }
  temporary.published();
}

} // namespace

int main(int argc, char *argv[]) {
  namespace po = boost::program_options;
  const auto description = options();
  po::variables_map arguments;
  try {
    po::store(po::parse_command_line(argc, argv, description), arguments);
    po::notify(arguments);
  } catch (const std::exception &error) {
    std::cerr << "invalid command line: " << error.what() << std::endl;
    return 2;
  }

  if (arguments.count("help") != 0u) {
    std::cout << description << std::endl;
    return 0;
  }
  if (arguments.count("version") != 0u) {
    std::cout << ypc::get_ypc_version() << std::endl;
    return 0;
  }
  if (arguments.count("gen-example-input") != 0u) {
    try {
      input_param_t example;
      example.set<parser_module_id>(kDefaultModule);
      ypc::ntjson::to_json_file(
          example, arguments["gen-example-input"].as<std::string>());
      return 0;
    } catch (const std::exception &error) {
      std::cerr << "cannot generate example input: " << error.what()
                << std::endl;
      return 2;
    }
  }
  if (arguments.count("input") == 0u || arguments.count("output") == 0u) {
    std::cerr << "--input and --output are required" << std::endl;
    return 2;
  }

  try {
    const std::string input_path = arguments["input"].as<std::string>();
    input_param_t input =
        ypc::ntjson::from_json_file<input_param_t>(input_path);

    const std::string requested = input.get<parser_module_id>();
    const std::string module_id =
        requested.empty() ? kDefaultModule : requested;
    input.set<parser_module_id>(module_id);

    std::vector<std::string> registry_directories;
    const bool has_additional_registry =
        arguments.count("parser-registry-dir") != 0u;
    if (boost::filesystem::exists(FID_DEFAULT_PARSER_REGISTRY_DIR)) {
      registry_directories.push_back(FID_DEFAULT_PARSER_REGISTRY_DIR);
    } else if (!has_additional_registry) {
      throw std::runtime_error(
          std::string("default parser registry directory does not exist: ") +
          FID_DEFAULT_PARSER_REGISTRY_DIR);
    } else {
      std::cerr << "default parser registry is absent; using explicit "
                   "build-tree registries only"
                << std::endl;
    }
    if (has_additional_registry) {
      const auto additions =
          arguments["parser-registry-dir"].as<std::vector<std::string>>();
      registry_directories.insert(registry_directories.end(), additions.begin(),
                                  additions.end());
    }

    fid::parser_registry registry;
    registry.load(registry_directories);
    const auto selection = registry.select(module_id, input.get<parser_path>(),
                                           input.get<parser_enclave_hash>());

    plugin_parser parser(input, selection);
    std::cout << "start to parse with module " << module_id << std::endl;
    const uint32_t status = parser.parse();
    if (status != ypc::success) {
      std::cerr << "analysis failed with status " << status << " ("
                << ypc::status_string(status) << ")" << std::endl;
      return 4;
    }

    write_output_atomically(arguments["output"].as<std::string>(),
                            parser.get_result_str());
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "fid_analyzer failed: " << error.what() << std::endl;
    return 3;
  } catch (...) {
    std::cerr << "fid_analyzer failed with an unknown error" << std::endl;
    return 3;
  }
}
