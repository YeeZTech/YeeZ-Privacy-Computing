#pragma once
#include <hpda/common/common.h>
#include <hpda/common/processor_with_output.h>

namespace hpda {
namespace extractor {
namespace internal {
template <typename OutputObjType>
class extractor_base
    : public ::hpda::internal::processor_with_output<OutputObjType> {
public:
  virtual ~extractor_base() {}
};
} // namespace internal

} // namespace extractor
} // namespace hpda
