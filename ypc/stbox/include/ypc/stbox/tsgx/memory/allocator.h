#pragma once
#include <memory>

namespace stbox{
struct default_user_allocator_malloc_free {
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  inline static void * malloc(size_type bytes){ return std::malloc(bytes); }
  inline static void free(char * const block){ std::free(block);  }
};

}
