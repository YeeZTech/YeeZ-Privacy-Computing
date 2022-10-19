#pragma once
#include <hpda/common/common.h>
#include <limits>
#include <type_traits>

namespace hpda {
template <typename PointType, typename DistanceType> struct euclidean {
  static DistanceType distance_square(const PointType &p1,
                                      const PointType &p2) {
    static_assert(sizeof(PointType) == -1,
                  "You have to have a specialization for euclidean distance "
                  "your point type");
    return DistanceType();
  }
};

namespace algorithm {
namespace kmeans {
template <typename PointIteratorType, typename PointType>
struct direct_point_traits {
public:
  static PointType point(const PointIteratorType &p) { return *p; }
};

namespace internal {

template <typename PointIteratorType, typename PointType, typename DistanceType,
          typename PointTraitsType =
              direct_point_traits<PointIteratorType, PointType>,
          typename PointContainer = std::vector<PointType>,
          bool PointContainerChecker = std::is_same<
              typename PointContainer::value_type, PointType>::value>
class loyd_impl {
public:
  typedef PointContainer points_t;
  loyd_impl(const PointIteratorType &points_begin,
            const PointIteratorType &points_end, const points_t &initial, int k,
            DistanceType delta)
      : m_begin(points_begin), m_end(points_end), m_means(initial), m_k(k),
        m_delta(delta * delta) {
    for (int i = 0; i < k; ++i) {
      m_last_means.push_back(PointType());
    }
  }

  void run() {
    if (m_means.size() != m_k) {
      throw std::runtime_error("loyd, inconsistent point number");
    }
    for (int i = 0; i < m_k; ++i) {
      m_clusters.push_back(point_refs_t());
    }
    m_round = 0;
    while (!is_all_means_close_enough()) {

      m_round++;
      for (int i = 0; i < m_k; ++i) {
        m_clusters[i].clear();
      }

      for (PointIteratorType i = m_begin; i != m_end; i++) {
        assign(i);
      }

      m_last_means = m_means;
      for (int i = 0; i < m_k; ++i) {
        PointType p;
        for (int j = 0; j < m_clusters[i].size(); ++j) {
          p = p + PointTraitsType::point(m_clusters[i][j]);
        }
        p = p / m_clusters[i].size();
        m_means[i] = p;
      }
    }
  }

  points_t means() const { return m_means; }
  int round() const { return m_round; }

  // DistanceType average_distance() const{}

  std::vector<std::vector<PointIteratorType>> &clusters() { return m_clusters; }

protected:
  bool is_all_means_close_enough() {
    for (int i = 0; i < m_k; i++) {
      DistanceType d = euclidean<PointType, DistanceType>::distance_square(
          m_means[i], m_last_means[i]);
      if (d >= m_delta) {
        return false;
      }
    }
    return true;
  }

  void assign(const PointIteratorType &p) {
    DistanceType min = std::numeric_limits<DistanceType>::max();
    int index;
    for (int i = 0; i < m_means.size(); ++i) {
      auto d = euclidean<PointType, DistanceType>::distance_square(
          PointTraitsType::point(p), m_means[i]);
      if (d < min) {
        index = i;
        min = d;
      }
    }
    m_clusters[index].push_back(p);
  }

protected:
  PointIteratorType m_begin;
  PointIteratorType m_end;
  int m_k;
  DistanceType m_delta;
  int m_round;

  typedef std::vector<PointIteratorType> point_refs_t;
  std::vector<point_refs_t> m_clusters;
  points_t m_means;
  points_t m_last_means;
};

template <typename PointIteratorType, typename PointType, typename DistanceType,
          typename PointTraitsType, typename PointContainer>
class loyd_impl<PointIteratorType, PointType, DistanceType, PointTraitsType,
                PointContainer, false> {
  static_assert(
      sizeof(PointIteratorType) == -1,
      "point container type should be for point type, not other type");
};

} // namespace internal
} // namespace kmeans
} // namespace algorithm
} // namespace hpda
