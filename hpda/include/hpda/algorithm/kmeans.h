#pragma once
#include <hpda/algorithm/internal/kmeans_loyd.h>
#include <hpda/common/processor_with_input.h>
#include <hpda/extractor/extractor_base.h>
#include <hpda/extractor/raw_data.h>

namespace hpda {

namespace algorithm {
namespace kmeans {
template <typename PointType, typename DistanceType> struct kmeans_traits {
  define_nt(mean_point, PointType);
  define_nt(average_distance, DistanceType);
};

template <typename PointFlag,
          typename PointContainer =
              std::vector<typename ::ff::util::type_of_nt<PointFlag>::type>>
struct initial_point_picker_first_k {
  template <typename AllPointsType>
  static PointContainer points(const AllPointsType &all_points, int k) {
    PointContainer pc;
    auto it = all_points.begin();
    for (int i = 0; i < k && i < all_points.size(); ++i) {
      pc.push_back((*it).template get<PointFlag>());
      it++;
    }
    return pc;
  }
};

template <typename PointFlag,
          typename PointContainer =
              std::vector<typename ::ff::util::type_of_nt<PointFlag>::type>>
struct initial_point_picker_even_k {
  template <typename AllPointsType>
  static PointContainer points(const AllPointsType &all_points, int k) {
    PointContainer pc;
    int t = all_points.size() / k;
    for (int i = 0; i < k; ++i) {
      auto pos = t * i % all_points.size();
      auto &it = all_points[pos];
      pc.push_back(it.template get<PointFlag>());
    }
    return pc;
  }
};

namespace internal {

template <typename InputObjType, typename PointFlag, typename DistanceType,
          typename ClassifiedID,
          typename InitialPointPicker = initial_point_picker_even_k<PointFlag>>
class loyd_kmeans_impl
    : public ::hpda::internal::processor_with_input<InputObjType> {
public:
  loyd_kmeans_impl(
      ::hpda::internal::processor_with_output<InputObjType> *upper_stream,
      int k, DistanceType delta,
      int max_points = std::numeric_limits<int>::max())
      : ::hpda::internal::processor_with_input<InputObjType>(upper_stream),
        m_calculate_flag(false), m_k(k), m_delta(delta),
        m_max_points(max_points) {}

  typedef ::hpda::internal::processor_with_input<InputObjType> base;
  typedef loyd_kmeans_impl<InputObjType, PointFlag, DistanceType, ClassifiedID,
                           InitialPointPicker>
      self_type;

  typedef typename ::ff::util::type_of_nt<PointFlag>::type point_type;

  typedef
      typename kmeans_traits<point_type, DistanceType>::mean_point mean_point;
  typedef typename kmeans_traits<point_type, DistanceType>::average_distance
      average_distance;
  typedef ntobject<ClassifiedID, mean_point, average_distance>
      means_stream_output_type;

  typedef ::hpda::extractor::internal::raw_data_impl<means_stream_output_type>
      means_stream_t;
  typedef ::hpda::extractor::internal::raw_data_impl<
      typename ::ff::util::append_type<InputObjType, ClassifiedID>::type>
      data_with_cluster_stream_t;

  template <typename PointContainerType> struct point_traits {
  public:
    static point_type point(const typename PointContainerType::iterator &p) {
      return (*p).template get<PointFlag>();
    }
  };

  std::vector<InputObjType> m_all_points;
  virtual bool process() {
    // int point_count = 0;
    if (base::has_input_value() && m_all_points.size() < m_max_points) {
      m_all_points.push_back(base::input_value());
      return false;
    }
    // while (base::next_input() && point_count < m_max_points) {
    // all_points.push_back(base::input_value());
    // point_count++;
    //}
    // if (all_points.size() < m_k) {
    // return;
    //}

    using loyd_impl_type =
        loyd_impl<typename std::vector<InputObjType>::iterator, point_type,
                  DistanceType, point_traits<std::vector<InputObjType>>,
                  std::vector<point_type>>;

    auto initial_points = InitialPointPicker::points(m_all_points, m_k);

    loyd_impl_type lit(m_all_points.begin(), m_all_points.end(), initial_points,
                       m_k, m_delta);
    lit.run();
    m_all_points.clear();

    typedef typename ::ff::util::append_type<InputObjType, ClassifiedID>::type
        output_obj_type;
    if (m_cluster_stream) {
      auto &clusters = lit.clusters();
      for (int i = 0; i < lit.clusters().size(); ++i) {
        auto &cluster = lit.clusters()[i];
        for (auto it : cluster) {
          // TODO optimize for rvalue
          output_obj_type ot =
              ::ff::util::append_type<InputObjType, ClassifiedID>::value(*it,
                                                                         i);
          m_cluster_stream->add_data(std::move(ot));
        }
      }
    }

    if (m_means_stream) {
      auto means = lit.means();
      for (int i = 0; i < means.size(); ++i) {
        means_stream_output_type ot;
        ot.template set<ClassifiedID>(i);
        ot.template set<mean_point>(means[i]);
        ot.template set<average_distance>(0);
        m_means_stream->add_data(std::move(ot));
      }
    }
    m_calculate_flag = true;
  }
  /*
  class data_with_cluster_stream_t
      : public ::hpda::extractor::internal::raw_data_impl<
            typename ::ff::util::append_type<InputObjType,
                                             ClassifiedID>::type> {
  public:
    data_with_cluster_stream_t(self_type *self)
        : ::hpda::extractor::internal::raw_data_impl<
              typename ::ff::util::append_type<InputObjType,
                                               ClassifiedID>::type>(),
          m_self(self){};

    typedef ::hpda::extractor::internal::raw_data_impl<
        typename ::ff::util::append_type<InputObjType, ClassifiedID>::type>
        cluster_stream_base_type;

    virtual bool process() {
      m_self->calculate();
      return true;
      // return cluster_stream_base_type::next_output();
    }

  protected:
    self_type *m_self;
  };

  class means_stream_t : public ::hpda::extractor::internal::raw_data_impl<
                             means_stream_output_type> {
  public:
    means_stream_t(self_type *self)
        : ::hpda::extractor::internal::raw_data_impl<
              means_stream_output_type>(),
          m_self(self){};

    typedef ::hpda::extractor::internal::raw_data_impl<means_stream_output_type>
        means_stream_base_type;

    virtual bool process() {
      m_self->calculate();
      return true;
    }

  protected:
    self_type *m_self;
  };*/

  data_with_cluster_stream_t *data_with_cluster_stream() {
    if (!m_cluster_stream) {
      if (m_calculate_flag) {
        throw std::runtime_error(
            "you should call data_with_cluster_stream() in "
            "kmeans before dump the output");
      }
      m_cluster_stream.reset(new data_with_cluster_stream_t());
    }
    return m_cluster_stream.get();
  };

  means_stream_t *means_stream() {
    if (!m_means_stream) {
      if (m_calculate_flag) {
        throw std::runtime_error(
            "you should call means_stream() in kmeans before dump the output");
      }
      m_means_stream.reset(new means_stream_t());
    }
    return m_means_stream.get();
  }

protected:

  std::unique_ptr<data_with_cluster_stream_t> m_cluster_stream;
  std::unique_ptr<means_stream_t> m_means_stream;
  bool m_calculate_flag;
  int m_k;
  DistanceType m_delta;
  int m_max_points;
};
} // namespace internal

template <typename InputObjType, typename PointFlag, typename DistanceType,
          typename ClassifiedID>
using kmeans_processor = internal::loyd_kmeans_impl<InputObjType, PointFlag,
                                                    DistanceType, ClassifiedID>;

} // namespace kmeans

} // namespace algorithm
} // namespace hpda
