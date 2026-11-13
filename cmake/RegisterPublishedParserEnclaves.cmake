cmake_minimum_required(VERSION 3.12)

# Offline registration for already-published Fidelius parser enclaves.
#
# fid_analyzer only runs a signed enclave that the parser registry vouches for,
# and an enclave's contract is frozen when it is signed.  Enclaves built from
# source register themselves at build time (see fid_mark_parser_enclave and
# enclave_sign); this script covers the other case -- binaries that were signed
# and shipped earlier and must not be rebuilt or re-signed.  It only reads the
# signed files, hashes them, and asks sgx_sign to dump their metadata.
#
# Which enclave belongs to which contract is a deployment-specific assertion,
# so the catalogue lives with the business project, not here.  Point this
# script at that catalogue:
#
#   cmake -DFID_ENCLAVE_DIR=<dir with *.signed.so> \
#         -DFID_REGISTRY_OUTPUT_DIR=<prefix>/share/fidelius/parser-registry.d \
#         -DFID_ENCLAVE_MANIFEST=<project>/published_enclaves.cmake \
#         -P RegisterPublishedParserEnclaves.cmake
#
# The manifest is plain CMake and may call:
#
#   fid_parser_contract(<out_var> EDL <edl-file>)
#       fingerprint of a custom EDL contract; a relative path is resolved
#       against the manifest's own directory, then against this script's
#       directory (where projects install their contracts).
#   fid_parser_standard_contract(<out_var>)
#       fingerprint of the stock eparser.edl behind fidelius.eparser.v1.
#   fid_register_published_enclave(<name> <module-id> <fingerprint>)
#       register <FID_ENCLAVE_DIR>/<name>.signed.so under that contract.
#       A missing file is a warning, not an error, so a deployment that
#       carries only part of a catalogue can still register the rest.
#
# Optional overrides: SGX_SIGNER, FID_REGISTRY_GENERATOR,
# FID_EDL_FINGERPRINT_HELPER.

set(FID_REGISTRY_TOOL_DIR "${CMAKE_CURRENT_LIST_DIR}")

foreach(required FID_ENCLAVE_DIR FID_REGISTRY_OUTPUT_DIR FID_ENCLAVE_MANIFEST)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "${required} is required")
  endif()
endforeach()

if(NOT IS_DIRECTORY "${FID_ENCLAVE_DIR}")
  message(FATAL_ERROR
    "Published enclave directory does not exist: ${FID_ENCLAVE_DIR}")
endif()
if(NOT EXISTS "${FID_ENCLAVE_MANIFEST}")
  message(FATAL_ERROR "Enclave manifest does not exist: ${FID_ENCLAVE_MANIFEST}")
endif()
get_filename_component(FID_MANIFEST_DIR "${FID_ENCLAVE_MANIFEST}" DIRECTORY)

if(NOT DEFINED SGX_SIGNER OR "${SGX_SIGNER}" STREQUAL "")
  if(DEFINED ENV{SGX_SDK} AND EXISTS "$ENV{SGX_SDK}/bin/x64/sgx_sign")
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

# The framework pieces sit either beside this script's installed location
# (<prefix>/share/fidelius/registry-tools -> <prefix>/lib/cmake/YPC) or in the
# source tree it was authored in.
function(_fid_locate output description)
  foreach(candidate ${ARGN})
    if(EXISTS "${candidate}")
      get_filename_component(candidate "${candidate}" REALPATH)
      set(${output} "${candidate}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  message(FATAL_ERROR "${description} was not found; set ${output}")
endfunction()

if(NOT DEFINED FID_REGISTRY_GENERATOR OR "${FID_REGISTRY_GENERATOR}" STREQUAL "")
  _fid_locate(FID_REGISTRY_GENERATOR "GenerateFidParserRegistry.cmake"
    "${FID_REGISTRY_TOOL_DIR}/../../../lib/cmake/YPC/GenerateFidParserRegistry.cmake"
    "${FID_REGISTRY_TOOL_DIR}/../toolkit/analyzer/cmake/GenerateFidParserRegistry.cmake")
endif()
if(NOT DEFINED FID_EDL_FINGERPRINT_HELPER OR
   "${FID_EDL_FINGERPRINT_HELPER}" STREQUAL "")
  _fid_locate(FID_EDL_FINGERPRINT_HELPER "FidEdlFingerprint.cmake"
    "${FID_REGISTRY_TOOL_DIR}/../../../lib/cmake/YPC/FidEdlFingerprint.cmake"
    "${FID_REGISTRY_TOOL_DIR}/FidEdlFingerprint.cmake")
endif()
include("${FID_EDL_FINGERPRINT_HELPER}")

_fid_locate(FID_FIDELIUS_EDL "the Fidelius eparser.edl contract"
  "${FID_REGISTRY_TOOL_DIR}/../../../include/ypc/edl/eparser.edl"
  "${FID_REGISTRY_TOOL_DIR}/../include/ypc/edl/eparser.edl")
get_filename_component(FID_FIDELIUS_EDL_DIR "${FID_FIDELIUS_EDL}" DIRECTORY)

set(FID_EDL_SEARCH_PATHS
  "${FID_FIDELIUS_EDL_DIR}/../.."
  "${FID_FIDELIUS_EDL_DIR}"
  "${FID_FIDELIUS_EDL_DIR}/util"
  "${FID_FIDELIUS_EDL_DIR}/../stbox")
if(DEFINED ENV{SGX_SDK} AND IS_DIRECTORY "$ENV{SGX_SDK}/include")
  list(APPEND FID_EDL_SEARCH_PATHS "$ENV{SGX_SDK}/include")
endif()
if(IS_DIRECTORY "/opt/intel/sgxsdk/include")
  list(APPEND FID_EDL_SEARCH_PATHS "/opt/intel/sgxsdk/include")
endif()

function(fid_parser_contract output)
  cmake_parse_arguments(FID "" "EDL" "" ${ARGN})
  if(NOT FID_EDL)
    message(FATAL_ERROR "fid_parser_contract: EDL is required")
  endif()
  set(edl_file "")
  foreach(candidate
      "${FID_EDL}"
      "${FID_MANIFEST_DIR}/${FID_EDL}"
      "${FID_REGISTRY_TOOL_DIR}/${FID_EDL}")
    if(EXISTS "${candidate}" AND NOT IS_DIRECTORY "${candidate}")
      set(edl_file "${candidate}")
      break()
    endif()
  endforeach()
  if(edl_file STREQUAL "")
    message(FATAL_ERROR "EDL contract was not found: ${FID_EDL}")
  endif()
  fid_compute_edl_fingerprint(fingerprint "${edl_file}"
    ${FID_EDL_SEARCH_PATHS})
  set(${output} "${fingerprint}" PARENT_SCOPE)
endfunction()

function(fid_parser_standard_contract output)
  fid_compute_edl_fingerprint(fingerprint "${FID_FIDELIUS_EDL}"
    ${FID_EDL_SEARCH_PATHS})
  set(${output} "${fingerprint}" PARENT_SCOPE)
endfunction()

function(fid_register_published_enclave enclave_name module_id edl_fingerprint)
  if("${edl_fingerprint}" STREQUAL "")
    message(FATAL_ERROR
      "fid_register_published_enclave(${enclave_name}): empty EDL fingerprint")
  endif()
  set(enclave_file "${FID_ENCLAVE_DIR}/${enclave_name}.signed.so")
  # A deployment may carry only part of the catalogue. Skipping keeps the rest
  # registrable; an unregistered enclave is still refused at run time.
  if(NOT EXISTS "${enclave_file}")
    message(WARNING "Published enclave is missing, skipped: ${enclave_file}")
    return()
  endif()
  execute_process(
    COMMAND "${CMAKE_COMMAND}"
      "-DFID_REGISTRY_KIND=ENCLAVE"
      "-DFID_REGISTRY_OUTPUT=${FID_REGISTRY_OUTPUT_DIR}/${enclave_name}.enclave.json"
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

include("${FID_ENCLAVE_MANIFEST}")

message(STATUS
  "Registered published parser enclaves in ${FID_REGISTRY_OUTPUT_DIR}")
