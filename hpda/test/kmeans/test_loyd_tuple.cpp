#include "tuple_point.h"
#include <gtest/gtest.h>
#include <hpda/algorithm/internal/kmeans_loyd.h>
#include <iostream>
#include <random>
#include <unordered_set>


TEST(kmeans_loyd, tuple) {
  typedef std::vector<Point> Points;
  using loyd_impl =
      hpda::algorithm::kmeans::internal::loyd_impl<Points::iterator, Point,
                                                   double>;

  Points ps;
  std::uniform_real_distribution<double> unif(0, 1000);
  std::default_random_engine re;
  for (int i = 0; i < 1000; i++) {
    Point p;
    std::get<0>(p) = unif(re);
    std::get<1>(p) = unif(re);
    std::get<2>(p) = unif(re);
    ps.push_back(p);
  }

  Points inital;

  int k = 5;
  std::unordered_set<int> choosed;
  for (int i = 0; i < k; ++i) {
    int ti = rand() % ps.size();
    while (choosed.find(ti) != choosed.end()) {
      ti = rand() % ps.size();
    }
    choosed.insert(ti);
    inital.push_back(ps[i]);
  }

  loyd_impl li(ps.begin(), ps.end(), inital, k, 0.1);

  li.run();

  for (int i = 0; i < inital.size(); ++i) {
    std::cout << "initial " << i << ": " << inital[i] << std::endl;
  }

  std::cout << "rounds: " << li.round() << std::endl;
  for (int i = 0; i < li.means().size(); ++i) {
    std::cout << i << ": " << li.means()[i] << std::endl;
  }
}
