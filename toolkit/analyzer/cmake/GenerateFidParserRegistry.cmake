cmake_minimum_required(VERSION 3.12)

function(_fid_json_escape input output)
  string(REPLACE "\\" "\\\\" value "${input}")
  string(REPLACE "\"" "\\\"" value "${value}")
  set(${output} "${value}" PARENT_SCOPE)
endfunction()

find_program(FID_CHMOD_EXECUTABLE NAMES chmod)
if(NOT FID_CHMOD_EXECUTABLE)
  message(FATAL_ERROR "chmod is required to secure parser registry files")
endif()
function(_fid_set_registry_mode path mode)
  execute_process(COMMAND "${FID_CHMOD_EXECUTABLE}" "${mode}" "${path}"
                  RESULT_VARIABLE chmod_result)
  if(NOT chmod_result EQUAL 0)
    message(FATAL_ERROR "Cannot set parser registry permissions on ${path}")
  endif()
endfunction()

foreach(required FID_REGISTRY_KIND FID_REGISTRY_OUTPUT MODULE_ID
                 EDL_FINGERPRINT)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "GenerateFidParserRegistry: ${required} is required")
  endif()
endforeach()

get_filename_component(registry_directory "${FID_REGISTRY_OUTPUT}" DIRECTORY)
file(MAKE_DIRECTORY "${registry_directory}")
_fid_set_registry_mode("${registry_directory}" 0755)
set(registry_temp "${FID_REGISTRY_OUTPUT}.tmp")
file(REMOVE "${registry_temp}")
_fid_json_escape("${MODULE_ID}" module_id_json)
_fid_json_escape("${EDL_FINGERPRINT}" fingerprint_json)

if(FID_REGISTRY_KIND STREQUAL "MODULE")
  foreach(required PLUGIN_FILE RELATIVE_LIBRARY_PATH)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
      message(FATAL_ERROR "GenerateFidParserRegistry: ${required} is required")
    endif()
  endforeach()
  if(NOT EXISTS "${PLUGIN_FILE}")
    message(FATAL_ERROR "Parser plugin does not exist: ${PLUGIN_FILE}")
  endif()
  file(SHA256 "${PLUGIN_FILE}" plugin_sha256)
  _fid_json_escape("${RELATIVE_LIBRARY_PATH}" library_path_json)
  file(WRITE "${registry_temp}"
    "{\n"
    "  \"schema_version\": 1,\n"
    "  \"type\": \"parser_module\",\n"
    "  \"module_id\": \"${module_id_json}\",\n"
    "  \"abi_major\": 1,\n"
    "  \"abi_minor\": 0,\n"
    "  \"edl_fingerprint\": \"${fingerprint_json}\",\n"
    "  \"relative_library_path\": \"${library_path_json}\",\n"
    "  \"library_sha256\": \"${plugin_sha256}\"\n"
    "}\n")
  file(RENAME "${registry_temp}" "${FID_REGISTRY_OUTPUT}")
  _fid_set_registry_mode("${FID_REGISTRY_OUTPUT}" 0644)
elseif(FID_REGISTRY_KIND STREQUAL "ENCLAVE")
  foreach(required ENCLAVE_FILE SGX_SIGNER ENCLAVE_NAME)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
      message(FATAL_ERROR "GenerateFidParserRegistry: ${required} is required")
    endif()
  endforeach()
  if(NOT EXISTS "${ENCLAVE_FILE}")
    message(FATAL_ERROR "Signed enclave does not exist: ${ENCLAVE_FILE}")
  endif()
  if(NOT EXISTS "${SGX_SIGNER}")
    message(FATAL_ERROR "SGX signer does not exist: ${SGX_SIGNER}")
  endif()

  file(SHA256 "${ENCLAVE_FILE}" enclave_sha256)
  set(dump_file "${FID_REGISTRY_OUTPUT}.sgx-dump")
  set(css_file "${FID_REGISTRY_OUTPUT}.sgx-css")
  execute_process(
    COMMAND "${SGX_SIGNER}" dump -enclave "${ENCLAVE_FILE}"
            -dumpfile "${dump_file}" -cssfile "${css_file}"
    RESULT_VARIABLE dump_result
    OUTPUT_VARIABLE dump_stdout
    ERROR_VARIABLE dump_stderr)
  if(NOT dump_result EQUAL 0 OR NOT EXISTS "${dump_file}")
    file(REMOVE "${dump_file}" "${css_file}")
    message(FATAL_ERROR
      "Unable to extract MRENCLAVE from ${ENCLAVE_FILE}: ${dump_stderr}")
  endif()
  file(READ "${dump_file}" dump_text)
  string(FIND "${dump_text}"
    "metadata->enclave_css.body.enclave_hash.m:" marker_offset)
  if(marker_offset EQUAL -1)
    file(REMOVE "${dump_file}" "${css_file}")
    message(FATAL_ERROR "MRENCLAVE marker missing in sgx_sign dump")
  endif()
  string(SUBSTRING "${dump_text}" ${marker_offset} 512 hash_section)
  string(REGEX MATCHALL "0x[0-9a-fA-F][0-9a-fA-F]" hash_bytes
         "${hash_section}")
  list(LENGTH hash_bytes hash_byte_count)
  if(hash_byte_count LESS 32)
    file(REMOVE "${dump_file}" "${css_file}")
    message(FATAL_ERROR "Incomplete MRENCLAVE in sgx_sign dump")
  endif()
  set(mrenclave "")
  foreach(index RANGE 0 31)
    list(GET hash_bytes ${index} byte)
    string(SUBSTRING "${byte}" 2 2 byte)
    string(APPEND mrenclave "${byte}")
  endforeach()
  string(TOLOWER "${mrenclave}" mrenclave)
  file(REMOVE "${dump_file}" "${css_file}")

  _fid_json_escape("${ENCLAVE_NAME}" enclave_name_json)
  file(WRITE "${registry_temp}"
    "{\n"
    "  \"schema_version\": 1,\n"
    "  \"type\": \"parser_enclave\",\n"
    "  \"module_id\": \"${module_id_json}\",\n"
    "  \"edl_fingerprint\": \"${fingerprint_json}\",\n"
    "  \"enclave_name\": \"${enclave_name_json}\",\n"
    "  \"signed_file_sha256\": \"${enclave_sha256}\",\n"
    "  \"mrenclave\": \"${mrenclave}\"\n"
    "}\n")
  file(RENAME "${registry_temp}" "${FID_REGISTRY_OUTPUT}")
  _fid_set_registry_mode("${FID_REGISTRY_OUTPUT}" 0644)
else()
  message(FATAL_ERROR
    "GenerateFidParserRegistry: unsupported kind ${FID_REGISTRY_KIND}")
endif()
