#include "corecommon/package.h"
#include "model.h"
#include "ypc/byte.h"
#include <iostream>

int main(int argc, char *argv[]) {

  means_t model;
  std::vector<mean_t> means;

  mean_t m1;
  iris_data_t md1;
  md1.set<sepal_len, sepal_wid, petal_len, petal_wid>(5.004082, 3.416327,
                                                      1.465306, 0.244898);

  m1.set<cid, iris_data, average_distance_t>("setosa", md1, 0);
  means.push_back(m1.make_copy());

  md1.set<sepal_len, sepal_wid, petal_len, petal_wid>(5.883607, 2.740984,
                                                      4.388525, 1.434426);
  m1.set<cid, iris_data, average_distance_t>("versicolor", md1.make_copy(), 0);
  means.push_back(m1.make_copy());

  md1.set<sepal_len, sepal_wid, petal_len, petal_wid>(6.864864, 3.067567,
                                                      5.735135, 2.059460);
  m1.set<cid, iris_data, average_distance_t>("virginica", md1.make_copy(), 0);
  means.push_back(m1.make_copy());

  model.set<mean>(means);
  typename ypc::cast_obj_to_package<means_t>::type pkg = model;
  auto ret = ypc::make_bytes<ypc::bytes>::for_package(pkg);
  std::cout << ret << std::endl;

  return 0;
}
