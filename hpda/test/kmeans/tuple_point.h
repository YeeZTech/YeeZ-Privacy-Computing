#pragma once
#include <hpda/common/common.h>
//#include <hpda/hpda.h>

typedef std::tuple<double, double, double> Point;

namespace hpda {
template <typename T1, typename T2> struct euclidean;

template <> struct euclidean<Point, double> {
  static double distance_square(const Point &p1, const Point &p2) {
    double ret = 0;

    auto v1 = std::get<0>(p1) - std::get<0>(p2);
    auto v2 = std::get<1>(p1) - std::get<1>(p2);
    auto v3 = std::get<2>(p1) - std::get<2>(p2);

    ret += v1 * v1;
    ret += v2 * v2;
    ret += v3 * v3;
    return ret;
  }
};
} // namespace hpda

Point operator+(const Point &p1, const Point &p2);

Point operator/(const Point &p1, size_t v);

std::ostream &operator<<(std::ostream &os, const Point &obj);

define_nt(iid, int);
