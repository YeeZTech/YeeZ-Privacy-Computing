
find_package(OpenSSL REQUIRED)
add_executable(hpda_gtest gtest_main.cpp
  test_basic.cpp
  extractor/test_https_extractor.cpp
  extractor/test_paged_https_extractor.cpp
  processor/test_filter.cpp
  processor/test_groupby.cpp
  processor/test_json_to_data_batch.cpp
  processor/test_concat.cpp
  processor/test_split.cpp
  processor/test_set.cpp
  kmeans/test_loyd_vector.cpp
  kmeans/test_loyd_tuple.cpp
  kmeans/test_kmeans_tuple.cpp
  kmeans/tuple_point.cpp
  kmeans/test_kmeans_iris.cpp
  )
target_link_libraries(hpda_gtest PRIVATE gtest_main glog hpda OpenSSL::SSL OpenSSL::Crypto)
