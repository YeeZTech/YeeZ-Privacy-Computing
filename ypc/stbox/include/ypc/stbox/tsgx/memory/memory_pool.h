#pragma once
#include <memory>

namespace stbox{

  class memory_pool{
    public:
      explicit memory_pool(size_t pagesize);

      static void *malloc(size_t len);
      static void free(void *ptr);

    protected:
      size_t pagesize;
  };

}
