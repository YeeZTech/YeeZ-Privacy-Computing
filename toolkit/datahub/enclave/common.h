#include "sgx_trts.h"
#include "sgx_tseal.h"

// uint32_t get_sealed_data_size(uint32_t encrypt_data_size);
// sgx_status_t seal_file_data(uint8_t *encrypt_data, uint32_t in_size,
// uint8_t *sealed_blob, uint32_t data_size);

size_t unsealed_data_len(const uint8_t *sealed_blob, size_t data_size);

sgx_status_t unseal_data(const uint8_t *sealed_blob, size_t data_size,
                         uint8_t *raw_data, size_t raw_size);
