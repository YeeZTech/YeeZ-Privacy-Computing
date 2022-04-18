#include "ypc/version.h"

namespace ypc {
  std::string get_ypc_version() {
    return version(0, 2, 0).to_string();
  }
}
