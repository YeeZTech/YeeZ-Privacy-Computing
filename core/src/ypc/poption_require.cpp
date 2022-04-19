#include "ypc/poption_require.h"
namespace ypc {
namespace internal {
bool opt::check(const boost::program_options::variables_map &vm,
                bool exit_if_fail) const {
  if (exit_if_fail && !vm.count(m_value)) {
    std::cerr << "missing '" << m_value << "'" << std::endl;
    exit(-1);
  }
  return vm.count(m_value);
}

} // namespace internal
} // namespace ypc
