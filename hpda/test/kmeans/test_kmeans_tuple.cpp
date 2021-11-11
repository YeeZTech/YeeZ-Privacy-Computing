#include "tuple_point.h"
#include <gtest/gtest.h>
#include <hpda/hpda.h>
#include <random>

define_nt(hash, std::string);
define_nt(point, Point);
typedef hpda::ntobject<hash, point> data_item_t;

TEST(kmeans, empty) {
  hpda::extractor::raw_data<hash, point> rd;
  hpda::engine engine;
  rd.set_engine(&engine);

  typedef hpda::algorithm::kmeans::kmeans_processor<data_item_t, point, double,
                                                    iid>
      kmeans_t;
  kmeans_t km(&rd, 5, 0.1);

  hpda::output::memory_output<hash, point, iid> mo(
      km.data_with_cluster_stream());

  hpda::output::memory_output<iid, kmeans_t::mean_point,
                              kmeans_t::average_distance>
      m2(km.means_stream());

  engine.run();
}

TEST(kmeans, massive_points) {
  hpda::extractor::raw_data<hash, point> rd;
  hpda::engine engine;
  rd.set_engine(&engine);
  typedef hpda::algorithm::kmeans::kmeans_processor<data_item_t, point, double,
                                                    iid>
      kmeans_t;

  std::uniform_real_distribution<double> unif(0, 1000);
  std::default_random_engine re;
  for (int i = 0; i < 1000; i++) {
    data_item_t di;
    Point p;
    std::get<0>(p) = unif(re);
    std::get<1>(p) = unif(re);
    std::get<2>(p) = unif(re);
    di.set<hash>(std::to_string(i));
    di.set<point>(p);
    rd.add_data(di);
  }

  kmeans_t km(&rd, 5, 0.1);

  hpda::output::memory_output<hash, point, iid> mo(
      km.data_with_cluster_stream());

  hpda::output::memory_output<iid, kmeans_t::mean_point,
                              kmeans_t::average_distance>
      m2(km.means_stream());
  engine.run();
}
