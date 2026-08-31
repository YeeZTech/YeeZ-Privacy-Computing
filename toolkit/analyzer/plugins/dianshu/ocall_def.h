#pragma once
#include "ypc/corecommon/package.h"
#include "ypc/stbox/ebyte.h"

enum class ParserType {
  PARSER_MP4,
  PARSER_CSV,
  PARSER_TSV,
  PARSER_PDF,
  PARSER_ZIP,
  PARSER_RAR,
  PARSER_MP4_CUT,
};

define_nt(parser_type, int);
define_nt(ocall_data, stbox::bytes);
typedef ff::util::ntobject<::parser_type, ::ocall_data> ocall_data_item_t;

template <typename T>
stbox::bytes construct_ocall_data(T type, const stbox::bytes &data) {
  typename ypc::cast_obj_to_package<ocall_data_item_t>::type p;
  p.set<::parser_type>(int(type));
  p.set<::ocall_data>(data);
  return ypc::make_bytes<stbox::bytes>::for_package(p);
}
