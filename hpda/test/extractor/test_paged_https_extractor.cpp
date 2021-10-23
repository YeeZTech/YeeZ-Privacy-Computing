#include <gtest/gtest.h>
#include <hpda/extractor/paged_https_extractor.h>
#include <iostream>

TEST(paged_https_extractor, basic) {
  hpda::extractor::paged_https_extractor phe(
      "https://asresearch.io", "/iris-data?", 3,
      [](const std::string &prefix, int next_index, int page_limit) {
        std::stringstream ss;
        ss << prefix << "index=" << next_index << "&limit=" << page_limit;
        return ss.str();
      },
      [](const boost::property_tree::ptree &tree) {
        return tree.get<int>("start");
      },
      [](const boost::property_tree::ptree &tree) {
        return tree.get<int>("end");
      });

  std::cout << "start data" << std::endl;
  while (phe.process()) {
    std::cout << phe.output_value() << std::endl;
  }
  std::cout << "end data" << std::endl;
}
