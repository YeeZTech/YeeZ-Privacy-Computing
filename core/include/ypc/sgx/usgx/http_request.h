#pragma once

#include <stdint.h>

extern "C" {
uint32_t ocall_http_request(const char *service, uint32_t service_size,
                            const char *param, uint32_t param_size, char *resp,
                            uint32_t resp_size);
}
