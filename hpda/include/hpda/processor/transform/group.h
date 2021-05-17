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

  void begin_group() { m_data = 0; }
  template <typename T> void group(const T &t) {
    m_data += t.get<InputObjType>();
  }
  void end_group() {}
  typename output_type::value_type output() const { return m_data; }

protected:
  typename OutputColumnType::value_type m_data;
};

template <typename InputColumnType, typename OutputColumnType>
class avg : public aggregator_base<InputColumnType, OutputColumnType> {};

template <typename InputColumnType, typename OutputColumnType>
class max : public aggregator_base<InputColumnType, OutputColumnType> {};

template <typename InputColumnType, typename OutputColumnType>
class min : public aggregator_base<InputColumnType, OutputColumnType> {};

template <typename InputColumnType, typename OutputColumnType>
class count : public aggregator_base<InputColumnType, OutputColumnType> {};

template <typename InputObjType, typename OutputObjType, typename GroupByType,
          typename AggregatorType>
class groupby_impl : public processor_base<InputObjType, OutputObjType> {
public:
  groupby_impl(
      ::hpda::internal::processor_with_output<InputObjType> *upper_stream,
      const GroupByType &group_column, const AggregatorType &aggregator)
      : processor_base<InputObjType, OutputObjType>(upper_stream) {}
  virtual ~groupby_impl() {}

  typedef processor_base<InputObjType, OutputObjType> base;

  virtual bool next_output() {
    while (base::next_input()) {
      GroupByType t = base::input_value().get<GroupByType>();
      m_last_group_id = t;
      m_aggregator.begin_group();
      m_aggregator.group(base::input_value());
      m_data = base::input_value();
      while (m_last_group_id == t && base::next_input()) {
        m_aggregator.group(base::input_value());
      }
      m_aggregator.end_group();
      m_data.set<AggregatorType::output_type>(m_aggregator.output());
      return true;
    }
    return false;
  }
  virtual OutputObjType output_value() { return m_data; }

protected:
  GroupByType m_last_group_id;
  OutputObjType m_value;
  AggregatorType m_aggregator;
};
} // namespace internal
} // namespace processor
} // namespace hpda
