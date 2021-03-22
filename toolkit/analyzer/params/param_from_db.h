#pragma once
#include "db/db.h"
#include "params/param_source.h"

class param_from_db : public param_source {

public:
  param_from_db(const std::string &url, const std::string &usrname,
                const std::string &passwd, const std::string &dbname,
                const std::string &request_hash);
  virtual void read_from_source();

protected:
  using request_db = toolkit::analyzer::request_db;
  std::unique_ptr<request_db> m_db;
  std::string m_request_hash;
};
