#pragma once
#include <sgx_dh.h>
#include <sgx_report.h>
#include <sgx_tseal.h>

//#define MESSAGE_EXCHANGE 0x0
#define ENCLAVE_TO_ENCLAVE_CALL 0x1

#pragma pack(push, 1)
typedef struct _secure_message_t {
  uint32_t session_id; // Session ID identifyting the session to which the
                       // message belongs
  sgx_aes_gcm_data_t message_aes_gcm_data;
} secure_message_t;

#pragma pack(pop)

//#if defined(__cplusplus)
// extern "C" {
//#endif

// int printf(const char *fmt, ...);

//#if defined(__cplusplus)
//}
//#endif
