#pragma once
#include <string>

enum message_type_t {
  msg_params,
  msg_secret_key,
};

typedef struct {
  size_t m_id;
  std::string m_msg;
  std::string m_ehash;
} forward_message_st;
