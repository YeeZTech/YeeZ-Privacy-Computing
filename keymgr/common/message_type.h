#pragma once
#include "stbox/ebyte.h"
#include <string>

enum message_type_t {
  msg_params,
  msg_secret_key,
};

typedef struct {
  size_t m_id;
  stbox::bytes m_msg;
  stbox::bytes m_ehash;
} forward_message_st;
