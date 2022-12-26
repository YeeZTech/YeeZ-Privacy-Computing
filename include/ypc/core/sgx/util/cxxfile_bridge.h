#pragma once
#include <cstdint>

extern "C" {

uint32_t fopen_ocall(const char *filename, std::size_t len, uint32_t mode);

void fclose_ocall(uint32_t stream);

void fflush_ocall(uint32_t stream);

void fread_ocall(void *ptr, std::size_t size, uint32_t stream);

void fwrite_ocall(const void *ptr, std::size_t size, uint32_t stream);

void fseek_ocall(uint32_t stream, const uint8_t *offset, uint8_t dir);

void ftell_ocall(uint32_t stream, uint8_t *offset);

uint8_t feof_ocall(uint32_t stream);
uint8_t fgood_ocall(uint32_t stream);
uint8_t ffail_ocall(uint32_t stream);
uint8_t fbad_ocall(uint32_t stream);
}

namespace ypc {
void init_sgx_cxxfile();
void shutdown_sgx_cxxfile();
} // namespace ypc
