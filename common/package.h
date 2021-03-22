#pragma once
#include <ff/net/middleware/ntpackage.h>

enum pkg_type {
  ctrl_pkg_id,
  register_data_meta_pkg_id,
  ack_pkg_id,
};

enum privacy_data_type {
  dt_file,
  dt_https,
};

define_nt(ctrl_code, uint32_t);
typedef ff::net::ntpackage<ctrl_pkg_id, ctrl_code> ctrl_pkg_t;

define_nt(data_type_c, uint8_t);
define_nt(data_id_c, std::string);
define_nt(data_desc_c, std::string);
define_nt(exec_parser_path_c, std::string);
define_nt(exec_parser_param_c, std::string);
define_nt(sealed_data_path_c, std::string);
typedef ff::net::ntpackage<register_data_meta_pkg_id, data_type_c, data_id_c,
                           data_desc_c, exec_parser_path_c, exec_parser_param_c,
                           sealed_data_path_c>
    register_data_meta_pkg_t;

define_nt(ack_type_c, uint8_t);
typedef ff::net::ntpackage<ack_pkg_id, ack_type_c> ack_pkg_t;
