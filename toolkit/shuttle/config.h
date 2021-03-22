#include <boost/program_options.hpp>
#include <glog/logging.h>

namespace toolkit {
namespace shuttle {

class configure {
public:
  configure();
  virtual ~configure();

  void parse_config_file(const std::string &file);

  std::string help_message() const;

  inline const std::string &data_type_header() const {
    return m_bc_data_type_header;
  };
  inline const std::string &data_parser_lib() const { return m_bc_data_parser; }
  inline const std::string &data_id() const { return m_bc_data_id; }
  inline const std::string &exec_parser_path() const {
    return m_exec_parser_path;
  }
  inline const std::string &exec_params() const { return m_exec_params; }
  inline const std::string &sealed_data_url() const {
    return m_sealed_data_url;
  }
  inline const std::string &data_desc() const { return m_bc_data_desc; }

protected:
  boost::program_options::options_description m_options;
  std::string m_mode;
  std::string m_sealed_data_url;
  std::string m_bc_data_type_header;
  std::string m_bc_data_parser;
  std::string m_bc_data_id;
  std::string m_bc_data_desc;
  std::string m_exec_parser_path;
  std::string m_exec_params;
};
} // namespace shuttle
} // namespace toolkit
