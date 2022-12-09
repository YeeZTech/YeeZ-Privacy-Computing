#include "ypc/core/poption_require.h"
namespace ypc {
namespace internal {
bool opt::check(const boost::program_options::variables_map &vm, // NOLINT
                bool exit_if_fail) const {
  if (exit_if_fail && !vm.count(m_value)) { // NOLINT
    std::cerr << "missing '" << m_value << "'" << std::endl;
    exit(-1);
  }
  return vm.count(m_value); // NOLINT
}

} // namespace internal
} // namespace ypc
