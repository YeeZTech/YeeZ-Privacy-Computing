cmake_minimum_required(VERSION 3.12)

# Offline registration for the already-published Dianshu parser enclaves.  This
# script only reads the signed files, hashes them, and asks sgx_sign to dump
# their metadata.  It never invokes a signing operation or writes the enclave.
foreach(required DIANSHU_ENCLAVE_DIR FID_REGISTRY_OUTPUT_DIR)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "${required} is required")
  endif()
endforeach()

if(NOT IS_DIRECTORY "${DIANSHU_ENCLAVE_DIR}")
  message(FATAL_ERROR
    "Dianshu enclave directory does not exist: ${DIANSHU_ENCLAVE_DIR}")
endif()

if(NOT DEFINED SGX_SIGNER OR "${SGX_SIGNER}" STREQUAL "")
  if(DEFINED ENV{SGX_SDK} AND
     EXISTS "$ENV{SGX_SDK}/bin/x64/sgx_sign")
    set(SGX_SIGNER "$ENV{SGX_SDK}/bin/x64/sgx_sign")
  elseif(EXISTS "/opt/intel/sgxsdk/bin/x64/sgx_sign")
    set(SGX_SIGNER "/opt/intel/sgxsdk/bin/x64/sgx_sign")
  else()
    message(FATAL_ERROR
      "SGX_SIGNER is required (or set SGX_SDK to an Intel SGX SDK)")
  endif()
endif()

if(NOT EXISTS "${SGX_SIGNER}")
  message(FATAL_ERROR "SGX signer does not exist: ${SGX_SIGNER}")
endif()

if(NOT DEFINED FID_REGISTRY_GENERATOR OR
   "${FID_REGISTRY_GENERATOR}" STREQUAL "")
  set(generator_candidates
    "${CMAKE_CURRENT_LIST_DIR}/../../../lib/cmake/YPC/GenerateFidParserRegistry.cmake"
    "${CMAKE_CURRENT_LIST_DIR}/../../cmake/GenerateFidParserRegistry.cmake")
  foreach(candidate ${generator_candidates})
    if(EXISTS "${candidate}")
      set(FID_REGISTRY_GENERATOR "${candidate}")
      break()
    endif()
  endforeach()
endif()
if(NOT DEFINED FID_REGISTRY_GENERATOR OR
   NOT EXISTS "${FID_REGISTRY_GENERATOR}")
  message(FATAL_ERROR
    "GenerateFidParserRegistry.cmake was not found; set FID_REGISTRY_GENERATOR")
endif()

set(edl_file "${CMAKE_CURRENT_LIST_DIR}/dianshu_parser.edl")
if(NOT EXISTS "${edl_file}")
  message(FATAL_ERROR "Dianshu EDL contract is missing: ${edl_file}")
endif()
if(NOT DEFINED FID_EDL_FINGERPRINT_HELPER OR
   "${FID_EDL_FINGERPRINT_HELPER}" STREQUAL "")
  set(fingerprint_helper_candidates
    "${CMAKE_CURRENT_LIST_DIR}/../../../lib/cmake/YPC/FidEdlFingerprint.cmake"
    "${CMAKE_CURRENT_LIST_DIR}/../../../../cmake/FidEdlFingerprint.cmake")
  foreach(candidate ${fingerprint_helper_candidates})
    if(EXISTS "${candidate}")
      set(FID_EDL_FINGERPRINT_HELPER "${candidate}")
      break()
    endif()
  endforeach()
endif()
if(NOT DEFINED FID_EDL_FINGERPRINT_HELPER OR
   NOT EXISTS "${FID_EDL_FINGERPRINT_HELPER}")
  message(FATAL_ERROR
    "FidEdlFingerprint.cmake was not found; set FID_EDL_FINGERPRINT_HELPER")
endif()
include("${FID_EDL_FINGERPRINT_HELPER}")

set(fidelius_edl_candidates
  "${CMAKE_CURRENT_LIST_DIR}/../../../include/ypc/edl"
  "${CMAKE_CURRENT_LIST_DIR}/../../../../include/ypc/edl")
foreach(candidate ${fidelius_edl_candidates})
  if(EXISTS "${candidate}/eparser.edl")
    set(fidelius_edl_dir "${candidate}")
    break()
  endif()
endforeach()
if(NOT fidelius_edl_dir)
  message(FATAL_ERROR "Fidelius EDL imports were not found")
endif()

set(sgx_edl_search_paths)
if(DEFINED ENV{SGX_SDK} AND IS_DIRECTORY "$ENV{SGX_SDK}/include")
  list(APPEND sgx_edl_search_paths "$ENV{SGX_SDK}/include")
endif()
if(IS_DIRECTORY "/opt/intel/sgxsdk/include")
  list(APPEND sgx_edl_search_paths "/opt/intel/sgxsdk/include")
endif()
fid_compute_edl_fingerprint(
  dianshu_edl_fingerprint "${edl_file}"
  "${fidelius_edl_dir}/../.."
  "${fidelius_edl_dir}"
  "${fidelius_edl_dir}/util"
  "${fidelius_edl_dir}/../stbox"
  ${sgx_edl_search_paths})
fid_compute_edl_fingerprint(
  standard_edl_fingerprint "${fidelius_edl_dir}/eparser.edl"
  "${fidelius_edl_dir}/../.."
  "${fidelius_edl_dir}"
  "${fidelius_edl_dir}/util"
  "${fidelius_edl_dir}/../stbox"
  ${sgx_edl_search_paths})

function(register_published_enclave enclave_name module_id edl_fingerprint)
  set(enclave_file
    "${DIANSHU_ENCLAVE_DIR}/${enclave_name}.signed.so")
  if(NOT EXISTS "${enclave_file}")
    message(FATAL_ERROR "Published enclave is missing: ${enclave_file}")
  endif()
  set(fragment
    "${FID_REGISTRY_OUTPUT_DIR}/${enclave_name}.enclave.json")
  execute_process(
    COMMAND "${CMAKE_COMMAND}"
      "-DFID_REGISTRY_KIND=ENCLAVE"
      "-DFID_REGISTRY_OUTPUT=${fragment}"
      "-DMODULE_ID=${module_id}"
      "-DEDL_FINGERPRINT=${edl_fingerprint}"
      "-DENCLAVE_FILE=${enclave_file}"
      "-DENCLAVE_NAME=${enclave_name}.signed.so"
      "-DSGX_SIGNER=${SGX_SIGNER}"
      -P "${FID_REGISTRY_GENERATOR}"
    RESULT_VARIABLE generator_result)
  if(NOT generator_result EQUAL 0)
    message(FATAL_ERROR "Failed to register ${enclave_file}")
  endif()
endfunction()

# These mappings are an administrator assertion about the EDL used to build
# already-published binaries. The script hashes and inspects them read-only;
# it never modifies or re-signs an enclave.
set(dianshu_contract_enclaves
  csv_evaluate_parser
  heatmap_csv_evaluate_parser
  heatmap_tsv_evaluate_parser
  pdf_evaluate_parser
  zip_evaluate_parser
  rar_evaluate_parser
  mp4_cut_evaluate_parser
  mp4_3frame_evaluate_parser
  mp4_20frame_evaluate_parser)
set(standard_contract_enclaves
  txt_evaluate_parser
  opt_txt_evaluate_parser
  json_evaluate_parser
  download_parser
  download_parser_generate_key
  download_parser_data_reencrypt
  download_parser_get_key
  datasafebox_reencrypt)

foreach(enclave_name ${dianshu_contract_enclaves})
  register_published_enclave("${enclave_name}" "dianshu.eparser.v1"
    "${dianshu_edl_fingerprint}")
endforeach()
foreach(enclave_name ${standard_contract_enclaves})
  register_published_enclave("${enclave_name}" "fidelius.eparser.v1"
    "${standard_edl_fingerprint}")
endforeach()

message(STATUS
  "Registered 17 published Dianshu parser enclaves in ${FID_REGISTRY_OUTPUT_DIR}")
