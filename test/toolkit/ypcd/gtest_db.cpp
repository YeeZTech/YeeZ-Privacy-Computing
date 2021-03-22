#include "toolkit/ypcd/db.h"
#include <gtest/gtest.h>

TEST(test_db, construstor) {
  auto ddb_ptr =
      ypc::construct_db_ptr<toolkit::ypcd::ypcd_db>("gtest.mysql.conf");
  EXPECT_TRUE(ddb_ptr.get());
  EXPECT_TRUE(ddb_ptr->db_engine_ptr());
}

TEST(test_db, create_tables) {
  auto ddb_ptr =
      ypc::construct_db_ptr<toolkit::ypcd::ypcd_db>("gtest.mysql.conf");
  ddb_ptr->create_tables();
  auto *db = ddb_ptr->db_engine_ptr();
  auto ret_data = toolkit::ypcd::privacy_data_table::select<data_id, s_name, ts,
                                                            extra_path>(db)
                      .eval();
  EXPECT_TRUE(ret_data.empty());
  auto ret_analytic =
      toolkit::ypcd::analytic_program_table::select<enclave_hash, data_hash,
                                                    s_name, path, ts>(db)
          .eval();
  EXPECT_TRUE(ret_analytic.empty());
  auto ret_history = toolkit::ypcd::analytic_history_table::select<
                         data_id, data_hash, enclave_hash, ts, s_status>(db)
                         .eval();
  EXPECT_TRUE(ret_history.empty());
  ddb_ptr->clear_tables();
}

TEST(test_db, clear_tables) {
  auto ddb_ptr =
      ypc::construct_db_ptr<toolkit::ypcd::ypcd_db>("gtest.mysql.conf");
  auto *db = ddb_ptr->db_engine_ptr();
  ddb_ptr->create_tables();
  auto ret_data = toolkit::ypcd::privacy_data_table::select<data_id, s_name, ts,
                                                            extra_path>(db)
                      .eval();
  EXPECT_TRUE(ret_data.empty());
  ddb_ptr->clear_tables();
  EXPECT_THROW((toolkit::ypcd::privacy_data_table::select<data_id, s_name, ts,
                                                          extra_path>(db)
                    .eval()),
               std::exception);
  EXPECT_THROW(ddb_ptr->clear_tables(), std::exception);
}
