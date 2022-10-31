#pragma once
#include <hpda/processor/processor_base.h>

namespace hpda {
namespace processor {
namespace internal {
template <typename InputColumnType, typename OutputColumnType>
class aggregator_base {};

template <typename InputColumnType, typename OutputColumnType>
class sum : public aggregator_base<InputColumnType, OutputColumnType> {
public:
  typedef InputColumnType input_type;
  typedef OutputColumnType output_type;

  sum() : m_data(), m_grouping(false) {}

  void begin_group() { m_data = 0; }
  template <typename T> void group(const T &t) {
    m_data += t.template get<InputColumnType>();
    m_grouping = true;
  }
  void end_group() { m_grouping = false; }
  typename ::ff::util::internal::nt_traits<output_type>::type output() const {
    return m_data;
  }
  bool grouping() { return m_grouping; }

protected:
  typename ::ff::util::internal::nt_traits<OutputColumnType>::type m_data;
  bool m_grouping;
};

template <typename InputColumnType, typename OutputColumnType>
class avg : public aggregator_base<InputColumnType, OutputColumnType> {
public:
  typedef InputColumnType input_type;
  typedef OutputColumnType output_type;

  avg() : m_data(), m_grouping(false) {}
  void begin_group() {
    m_data = 0;
    m_count = 0;
  }
  template <typename T> void group(const T &t) {
    m_data += t.template get<InputColumnType>();
    m_count += 1;
    m_grouping = true;
  }
  void end_group() {
    if (m_count == 0) {
      return;
    }
    m_data = m_data / m_count;
    m_grouping = false;
  }

  bool grouping() { return m_grouping; }
  typename ::ff::util::internal::nt_traits<output_type>::type output() const {
    return m_data;
  }

protected:
  typename ::ff::util::internal::nt_traits<OutputColumnType>::type m_data;
  size_t m_count;
  bool m_grouping;
};

template <typename InputColumnType, typename OutputColumnType>
class max : public aggregator_base<InputColumnType, OutputColumnType> {
public:
  typedef InputColumnType input_type;
  typedef OutputColumnType output_type;

  max() : m_data(), m_grouping(false) {}

  void begin_group() { m_data = 0; }
  template <typename T> void group(const T &t) {
    auto d = t.template get<InputColumnType>();
    m_data = m_data > d ? m_data : d;
    m_grouping = true;
  }
  void end_group() { m_grouping = false; }
  bool grouping() {
    return m_grouping;
  }
  typename ::ff::util::internal::nt_traits<output_type>::type output() const {
    return m_data;
  }

protected:
  typename ::ff::util::internal::nt_traits<OutputColumnType>::type m_data;
  bool m_grouping;
};

template <typename InputColumnType, typename OutputColumnType>
class min : public aggregator_base<InputColumnType, OutputColumnType> {
public:
  typedef InputColumnType input_type;
  typedef OutputColumnType output_type;

  min() : m_data(), m_grouping(false) {}
  void begin_group() { m_data = 0; }
  template <typename T> void group(const T &t) {
    auto d = t.template get<InputColumnType>();
    m_data = m_data < d ? m_data : d;
    m_grouping = true;
  }
  void end_group() { m_grouping = false; }
  bool grouping() { return m_grouping; }
  typename ::ff::util::internal::nt_traits<output_type>::type output() const {
    return m_data;
  }

protected:
  typename ::ff::util::internal::nt_traits<OutputColumnType>::type m_data;
  bool m_grouping;
};

template <typename InputColumnType, typename OutputColumnType>
class count : public aggregator_base<InputColumnType, OutputColumnType> {
public:
  typedef InputColumnType input_type;
  typedef OutputColumnType output_type;

  count() : m_data(), m_grouping(false) {}
  void begin_group() { m_data = 0; }
  template <typename T> void group(const T &t) {
    m_data += 1;
    m_grouping = true;
  }
  void end_group() { m_grouping = false; }
  bool grouping() { return m_grouping; }
  typename ::ff::util::internal::nt_traits<output_type>::type output() const {
    return m_data;
  }

protected:
  typename ::ff::util::internal::nt_traits<OutputColumnType>::type m_data;
  bool m_grouping;
};

template <typename InputObjType, typename OutputObjType, typename GroupByType,
          typename AggregatorType>
class groupby_impl : public processor_base<InputObjType, OutputObjType> {
public:
  groupby_impl(
      ::hpda::internal::processor_with_output<InputObjType> *upper_stream,
      const AggregatorType &aggregator)
      : processor_base<InputObjType, OutputObjType>(upper_stream),
        m_aggregator(aggregator), m_continue(false) {}
  virtual ~groupby_impl() {}

  typedef processor_base<InputObjType, OutputObjType> base;

  virtual bool process() {
    typename ::ff::util::internal::nt_traits<GroupByType>::type t;
    if (!base::has_input_value()) {
      if (m_aggregator.grouping()) {
        m_aggregator.end_group();
        m_data.template set<typename AggregatorType::output_type>(
            m_aggregator.output());
        return true;
      }
      return false;
    }
    t = base::input_value().template get<GroupByType>();
    if (m_aggregator.grouping()) {
      if (m_last_group_id == t) {
        m_aggregator.group(base::input_value());
        base::consume_input_value();
        return false;
      } else {
        m_aggregator.end_group();
        m_data.template set<typename AggregatorType::output_type>(
            m_aggregator.output());
        m_aggregator.begin_group();
        m_last_group_id = t;
        m_data.template set<GroupByType>(t);
        m_aggregator.group(base::input_value());
        base::consume_input_value();
        return true;
      }
    } else {
      m_aggregator.begin_group();
      m_last_group_id = t;
      m_data.template set<GroupByType>(t);
      m_aggregator.group(base::input_value());
      base::consume_input_value();
      return false;
    }
  }

  virtual OutputObjType output_value() {
    return m_data.make_copy();
  }

protected:
  typename ::ff::util::internal::nt_traits<GroupByType>::type m_last_group_id;
  OutputObjType m_data;
  AggregatorType m_aggregator;
  bool m_continue;
};
} // namespace internal
template <typename InputObjType, typename GroupByType, typename AggregatorType,
          typename... ARGS>
using groupby = internal::groupby_impl<InputObjType, ntobject<ARGS...>,
                                       GroupByType, AggregatorType>;

template <typename InputObjType, typename OutputObjType, typename GroupByType,
          typename AggregatorType>
using groupby_t = internal::groupby_impl<InputObjType, OutputObjType,
                                         GroupByType, AggregatorType>;

namespace group {
template <typename InputObjType, typename OutputObjType>
using sum = internal::sum<InputObjType, OutputObjType>;
template <typename InputObjType, typename OutputObjType>
using avg = internal::avg<InputObjType, OutputObjType>;
template <typename InputObjType, typename OutputObjType>
using max = internal::max<InputObjType, OutputObjType>;
template <typename InputObjType, typename OutputObjType>
using min = internal::min<InputObjType, OutputObjType>;
template <typename InputObjType, typename OutputObjType>
using count = internal::count<InputObjType, OutputObjType>;
}
} // namespace processor
} // namespace hpda
