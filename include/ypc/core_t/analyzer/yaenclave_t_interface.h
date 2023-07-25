#ifndef YAENCLAVE_T_INTERFACE_H__
#define YAENCLAVE_T_INTERFACE_H__

#include "sgx_edger8r.h" /* for sgx_ocall etc. */
#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

#include "sgx_eid.h"
#include "ypc/stbox/tsgx/channel/dh_cdef.h"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

uint64_t get_ypc_analyzer_version(void);

sgx_status_t SGX_CDECL km_session_request_ocall(uint32_t *retval,
                                                sgx_dh_msg1_t *dh_msg1,
                                                uint32_t *session_id);
sgx_status_t SGX_CDECL km_exchange_report_ocall(uint32_t *retval,
                                                sgx_dh_msg2_t *dh_msg2,
                                                sgx_dh_msg3_t *dh_msg3,
                                                uint32_t session_id);
sgx_status_t SGX_CDECL km_send_request_ocall(
    uint32_t *retval, uint32_t session_id, secure_message_t *req_message,
    size_t req_message_size, size_t max_payload_size,
    secure_message_t *resp_message, size_t resp_message_size);
sgx_status_t SGX_CDECL km_end_session_ocall(uint32_t *retval,
                                            uint32_t session_id);

sgx_status_t SGX_CDECL km_session_request_oram_ocall(uint32_t *retval,
                                                sgx_dh_msg1_t *dh_msg1,
                                                uint32_t *session_id);
sgx_status_t SGX_CDECL km_exchange_report_oram_ocall(uint32_t *retval,
                                                sgx_dh_msg2_t *dh_msg2,
                                                sgx_dh_msg3_t *dh_msg3,
                                                uint32_t session_id);
sgx_status_t SGX_CDECL km_send_request_oram_ocall(
    uint32_t *retval, uint32_t session_id, secure_message_t *req_message,
    size_t req_message_size, size_t max_payload_size,
    secure_message_t *resp_message, size_t resp_message_size);
sgx_status_t SGX_CDECL km_end_session_oram_ocall(uint32_t *retval,
                                            uint32_t session_id);


sgx_status_t SGX_CDECL sgx_oc_cpuidex(int cpuinfo[4], int leaf, int subleaf);
sgx_status_t SGX_CDECL sgx_thread_wait_untrusted_event_ocall(int *retval,
                                                             const void *self);
sgx_status_t SGX_CDECL sgx_thread_set_untrusted_event_ocall(int *retval,
                                                            const void *waiter);
sgx_status_t SGX_CDECL sgx_thread_setwait_untrusted_events_ocall(
    int *retval, const void *waiter, const void *self);
sgx_status_t SGX_CDECL sgx_thread_set_multiple_untrusted_events_ocall(
    int *retval, const void **waiters, size_t total);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
