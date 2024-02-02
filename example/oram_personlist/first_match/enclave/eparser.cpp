#include "first_match_parser.h"
#include "ypc/core_t/analyzer/algo_wrapper.h"
#include "ypc/core_t/analyzer/macro.h"
#include "ypc/corecommon/crypto/stdeth.h"

ypc::algo_wrapper<ypc::crypto::eth_sgx_crypto, ypc::oram_sealed_data_stream,
                  first_match_parser,
                  ypc::onchain_result<ypc::crypto::eth_sgx_crypto>>
    pw;

YPC_PARSER_IMPL(pw);

// uint32_t begin_parse_data_item() { 
//     stbox::bytes hash(get_enclave_hash_size()); 
//     get_enclave_hash(hash.data(), hash.size()); 
//     pw.set_enclave_hash(hash.data(), hash.size()); 
//     return pw.begin_parse_data_item(); 
// } 

// uint32_t end_parse_data_item() { 
//     return pw.end_parse_data_item(); 
// } 

// uint32_t get_enclave_hash_size() { 
//     return 32; 
// } 

// uint32_t get_enclave_hash(uint8_t *hash, uint32_t hash_size) { 
//     auto enclave_hash = stbox::get_enclave_hash(); 
//     memcpy(hash, enclave_hash.data(), hash_size); 
//     return SGX_SUCCESS; 
// } 

// uint32_t get_analyze_result_size() { 
//     return pw.get_analyze_result_size(); 
// } 

// uint32_t get_analyze_result(uint8_t *res, uint32_t res_size) { 
//     return pw.get_analyze_result(res, res_size); 
    
// } 

// uint32_t init_data_source(const uint8_t *data_source_info, uint32_t len) { 
//     return call_init_data_source_helper<decltype(pw)>::call( pw, data_source_info, len); 
// }

// uint32_t init_model(const uint8_t *model, uint32_t len) { 
//     return call_init_model_helper<decltype(pw)>::call(pw, model, len); 
// } 

// uint32_t get_parser_type() { 
//     return pw.get_parser_type(); 
// } 

// uint32_t parse_data_item(const uint8_t *input_param, uint32_t len) { 
//     return pw.parse_data_item(input_param, len); 
// }