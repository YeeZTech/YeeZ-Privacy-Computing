#include <gtest/gtest.h>
#include <iostream>
#include <random>
#include <unordered_set>

namespace hpda {
template <typename T1, typename T2> struct euclidean;

template <> struct euclidean<std::vector<double>, double> {
  static double distance_square(const std::vector<double> &p1,
                                const std::vector<double> &p2) {
    double ret = 0;
    if (p1.empty() && p2.empty()) {
      return ret;
    }

    if (p2.empty()) {
      for (int i = 0; i < p1.size(); ++i) {
        ret += p1[i] * p1[i];
      }
      return ret;
    }

    if (p1.empty()) {
      for (int i = 0; i < p2.size(); ++i) {
        ret += p2[i] * p2[i];
      }
      return ret;
    }

    if (p1.size() != p2.size()) {
      throw std::runtime_error("dismatch point");
    }
    for (int i = 0; i < p1.size(); ++i) {
      ret += (p1[i] - p2[i]) * (p1[i] - p2[i]);
    }
    return ret;
  }
};
}
std::vector<double> operator+(const std::vector<double> &p1,
                              const std::vector<double> &p2) {
  if (p1.empty()) {
    return p2;
  }
  if (p2.empty()) {
    return p1;
  }
  std::vector<double> s(p1);

  if (p1.size() != p2.size()) {
    throw std::runtime_error("dismatch point");
  }

  for (int i = 0; i < p1.size(); ++i) {
    s[i] += p2[i];
  }
  return s;
}

std::vector<double> operator/(const std::vector<double> &p1, size_t v) {
  std::vector<double> s(p1);
  for (int i = 0; i < s.size(); ++i) {
    s[i] = s[i] / v;
  }
  return s;
}

std::ostream &operator<<(std::ostream &os, const std::vector<double> &obj) {
  os << "(";
  for (int i = 0; i < obj.size(); ++i) {
    os << obj[i];
    if (i != obj.size() - 1) {
      os << ", ";
    }
  }
  os << ")";
  return os;
}

#include <hpda/algorithm/internal/kmeans_loyd.h>
TEST(kmeans_loyd, vector) {
  typedef std::vector<double> Point;
  typedef std::vector<Point> Points;
  using loyd_impl =
      hpda::algorithm::kmeans::internal::loyd_impl<Points::iterator, Point,
                                                   double>;

  Points ps;
  std::uniform_real_distribution<double> unif(0, 1000);
  std::default_random_engine re;
  for (int i = 0; i < 1000; i++) {
    Point p;
    p.push_back(unif(re));
    p.push_back(unif(re));
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
