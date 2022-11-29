#include "tuple_point.h"
#include <ostream>

Point operator+(const Point &p1, const Point &p2) {
  Point s(p1);

  std::get<0>(s) += std::get<0>(p2);
  std::get<1>(s) += std::get<1>(p2);
  std::get<2>(s) += std::get<2>(p2);
  return s;
}

Point operator/(const Point &p1, size_t v) {
  Point s(p1);

  std::get<0>(s) /= v;
  std::get<1>(s) /= v;
  std::get<2>(s) /= v;
  return s;
}

std::ostream &operator<<(std::ostream &os, const Point &obj) {
  os << "(";
  os << std::get<0>(obj) << ", ";
  os << std::get<1>(obj) << ", ";
  os << std::get<2>(obj);
  os << ")";
  return os;
}
