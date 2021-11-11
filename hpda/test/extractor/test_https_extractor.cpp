#include <gtest/gtest.h>
#include <hpda/extractor/internal/https_request.h>
#include <iostream>

TEST(https_request, basic) {
#if 0
  hpda::extractor::internal::https_request req("asresearch.io",
                                               "/iris-data?index=1&limit=3");
  //"/iris-data?index=0&limit=10");
  EXPECT_TRUE(req.result().size() != 0);
#endif
}
