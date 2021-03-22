#include "toolkit/ypcd/services.h"
#include <gtest/gtest.h>

TEST(test_services, register_data_meta_service) {
  auto ddb_ptr =
      ypc::construct_db_ptr<toolkit::ypcd::ypcd_db>("gtest.mysql.conf");
  ddb_ptr->create_tables();
  auto *db = ddb_ptr->db_engine_ptr();

  auto pkg = std::make_shared<register_data_meta_pkg_t>();
  pkg->set<data_type_c>(dt_file);
  pkg->set<data_id_c, data_desc_c>("base64str", "iris_data");
  pkg->set<exec_parser_path_c, exec_parser_param_c>("parser.signed.so",
                                                    "params");
  pkg->set<sealed_data_path_c>("data.sealed");

  auto ret_data = toolkit::ypcd::privacy_data_table::select<data_id, s_name, ts,
                                                            extra_path>(db)
                      .eval();
  EXPECT_TRUE(ret_data.empty());
  toolkit::ypcd::register_data_meta_service(pkg.get(), db);
  ret_data = toolkit::ypcd::privacy_data_table::select<data_id, s_name, ts,
                                                       extra_path>(db)
                 .eval();
  EXPECT_EQ(ret_data.size(), 1);
  EXPECT_EQ(ret_data[0].get<data_id>(), "base64str");
  EXPECT_EQ(ret_data[0].get<s_name>(), "iris_data");
  EXPECT_EQ(ret_data[0].get<ts>(), 0);
  EXPECT_EQ(ret_data[0].get<extra_path>(), "data.sealed");
  ddb_ptr->clear_tables();
}
