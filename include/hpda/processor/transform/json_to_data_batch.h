#pragma once
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <glog/logging.h>
#include <hpda/processor/processor_base.h>

namespace hpda {
namespace processor {
namespace internal {

template <typename OutputObjType>
class json_to_data_batch_impl
    : public processor_base<std::string, OutputObjType> {
public:
  typedef std::function<const boost::property_tree::ptree &(
      const boost::property_tree::ptree &)>
      data_item_picker_t;
  typedef std::function<OutputObjType(const std::string &item)>
      item_transformer_t;

  json_to_data_batch_impl(
      ::hpda::internal::processor_with_output<std::string> *upper_stream,
      const data_item_picker_t &data_item_picker,
      const item_transformer_t &item_transformer)
      : processor_base<std::string, OutputObjType>(upper_stream),
        m_func(data_item_picker), m_item_transformer(item_transformer),
        m_next_data_index(0) {}
  virtual ~json_to_data_batch_impl() {}

  typedef processor_base<std::string, OutputObjType> base;

  virtual bool process() {
    m_next_data_index++;
    if (m_next_data_index >= m_all_data.size()) {
      if (!parse_data()) {
        base::consume_input_value();
        return false;
      }
    }
    return true;
  }

  virtual OutputObjType output_value() { return m_all_data[m_next_data_index]; }

protected:
  bool parse_data() {
    m_next_data_index = 0;
    m_all_data.clear();
    if (!base::has_input_value()) {
      return false;
    }
    try {
      std::string v = base::input_value();
      std::stringstream ss;
      ss << v;
      boost::property_tree::ptree tree;
      boost::property_tree::read_json(ss, tree);
      auto child = m_func(tree);
      BOOST_FOREACH (boost::property_tree::ptree::value_type &v, child) {
        assert(v.first.empty()); // array elements have no names
        try {
          m_all_data.push_back(m_item_transformer(v.second.data()));
        } catch (std::exception &e) {
          // ignore this data
          continue;
        }
      }

      return true;
    } catch (std::exception &e) {
      LOG(INFO) << "parse_data got exception: " << e.what();
      return false;
    }
  }

protected:
  std::vector<OutputObjType> m_all_data;
  int m_next_data_index;
  data_item_picker_t m_func;
  item_transformer_t m_item_transformer;
};
} // namespace internal
template <typename... ARGS>
using json_to_data_batch = internal::json_to_data_batch_impl<ntobject<ARGS...>>;
} // namespace processor
} // namespace hpda
