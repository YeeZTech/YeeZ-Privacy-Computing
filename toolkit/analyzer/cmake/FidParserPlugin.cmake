include(CMakeParseArguments)

if(NOT DEFINED FID_PARSER_PLUGIN_CMAKE_DIR)
  set(FID_PARSER_PLUGIN_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}")
endif()
set(FID_PARSER_PLUGIN_VERSION_SCRIPT
    "${FID_PARSER_PLUGIN_CMAKE_DIR}/fid_parser_plugin.map")
set(FID_PARSER_REGISTRY_GENERATOR
    "${FID_PARSER_PLUGIN_CMAKE_DIR}/GenerateFidParserRegistry.cmake")

# FidEdlFingerprint is a framework build utility: it sits next to FindSGX in
# the source tree and lands flat beside this file once installed.
if(NOT DEFINED FID_PARSER_EDL_FINGERPRINT_HELPER)
  foreach(candidate
      "${FID_PARSER_PLUGIN_CMAKE_DIR}/FidEdlFingerprint.cmake"
      "${FID_PARSER_PLUGIN_CMAKE_DIR}/../../../cmake/FidEdlFingerprint.cmake")
    if(EXISTS "${candidate}")
      set(FID_PARSER_EDL_FINGERPRINT_HELPER "${candidate}")
      break()
    endif()
  endforeach()
endif()
if(NOT FID_PARSER_EDL_FINGERPRINT_HELPER OR
   NOT EXISTS "${FID_PARSER_EDL_FINGERPRINT_HELPER}")
  message(FATAL_ERROR
    "FidEdlFingerprint.cmake was not found; set FID_PARSER_EDL_FINGERPRINT_HELPER")
endif()

include("${FID_PARSER_EDL_FINGERPRINT_HELPER}")

# Module ID of the stock eparser contract shipped with YPC.  Framework code
# never names it; it only asks to have an enclave tagged.
set(FID_STANDARD_PARSER_MODULE_ID "fidelius.eparser.v1")

function(_fid_parser_runtime_target output)
  if(TARGET fid_parser_plugin_runtime)
    set(${output} fid_parser_plugin_runtime PARENT_SCOPE)
  elseif(TARGET YPC::fid_parser_plugin_runtime)
    set(${output} YPC::fid_parser_plugin_runtime PARENT_SCOPE)
  else()
    message(FATAL_ERROR
      "fid_parser_plugin_runtime target is unavailable; "
      "install/find YPC component parser_plugin")
  endif()
endfunction()

function(_fid_parser_library_filename target output)
  get_target_property(output_name ${target} OUTPUT_NAME)
  if(NOT output_name OR output_name STREQUAL "output_name-NOTFOUND")
    set(output_name "${target}")
  endif()
  get_target_property(debug_postfix ${target} DEBUG_POSTFIX)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND debug_postfix AND
     NOT debug_postfix STREQUAL "debug_postfix-NOTFOUND")
    string(APPEND output_name "${debug_postfix}")
  endif()
  set(${output}
      "${CMAKE_SHARED_LIBRARY_PREFIX}${output_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
      PARENT_SCOPE)
endfunction()

# Tag an enclave target that implements a parser EDL contract so add_sign_enclave
# can emit a registry fragment for it after signing.  Framework code calls this
# opportunistically; without this module an enclave simply stays unregistered.
function(fid_mark_parser_enclave target)
  set(one_value MODULE_ID EDL)
  set(multi_value EDL_SEARCH_PATHS)
  cmake_parse_arguments(FID "" "${one_value}" "${multi_value}" ${ARGN})
  if(NOT FID_EDL)
    message(FATAL_ERROR "fid_mark_parser_enclave: EDL is required")
  endif()
  if(NOT FID_MODULE_ID)
    set(FID_MODULE_ID "${FID_STANDARD_PARSER_MODULE_ID}")
  endif()
  set_target_properties(${target} PROPERTIES
    FID_PARSER_MODULE_ID "${FID_MODULE_ID}"
    FID_PARSER_EDL "${FID_EDL}"
    FID_PARSER_EDL_SEARCH_PATHS "${FID_EDL_SEARCH_PATHS}")
endfunction()

function(add_fid_parser_plugin)
  set(one_value TARGET MODULE_ID EDL)
  set(multi_value SRCS EDL_SEARCH_PATHS LINK_LIBRARIES)
  cmake_parse_arguments(FID "" "${one_value}" "${multi_value}" ${ARGN})
  foreach(required TARGET MODULE_ID EDL)
    if(NOT FID_${required})
      message(FATAL_ERROR "add_fid_parser_plugin: ${required} is required")
    endif()
  endforeach()
  if(NOT FID_SRCS)
    message(FATAL_ERROR "add_fid_parser_plugin: SRCS is required")
  endif()
  if(NOT EXISTS "${FID_EDL}")
    message(FATAL_ERROR "add_fid_parser_plugin: EDL not found: ${FID_EDL}")
  endif()
  if(NOT EXISTS "${FID_PARSER_EDL_FINGERPRINT_HELPER}" OR
     NOT EXISTS "${FID_PARSER_PLUGIN_VERSION_SCRIPT}" OR
     NOT EXISTS "${FID_PARSER_REGISTRY_GENERATOR}")
    message(FATAL_ERROR "Fidelius parser plugin CMake support is incomplete")
  endif()

  fid_compute_edl_fingerprint(
    edl_fingerprint "${FID_EDL}" ${FID_EDL_SEARCH_PATHS})
  _fid_parser_runtime_target(runtime_target)

  add_untrusted_library(${FID_TARGET} SHARED USE_PREFIX
    SRCS ${FID_SRCS}
    EDL ${FID_EDL}
    EDL_SEARCH_PATHS ${FID_EDL_SEARCH_PATHS}
    EDL_DEPENDS ${edl_fingerprint_DEPENDENCIES})
  target_link_libraries(${FID_TARGET} PRIVATE
    ${runtime_target} ${FID_LINK_LIBRARIES})
  target_compile_definitions(${FID_TARGET} PRIVATE
    "FID_PARSER_MODULE_ID=\"${FID_MODULE_ID}\""
    "FID_PARSER_EDL_CONTRACT_FINGERPRINT=\"${edl_fingerprint}\"")
  # In this tree the headers come from the source tree; in a project that
  # consumes an installed YPC only YPC_INCLUDE_DIR exists.
  set(plugin_include_dirs)
  if(YPC_INCLUDE_DIR)
    list(APPEND plugin_include_dirs "${YPC_INCLUDE_DIR}")
  endif()
  if(EXISTS "${PROJECT_SOURCE_DIR}/include")
    list(APPEND plugin_include_dirs "${PROJECT_SOURCE_DIR}/include")
  endif()
  if(plugin_include_dirs)
    target_include_directories(${FID_TARGET} PRIVATE ${plugin_include_dirs})
  endif()
  set_target_properties(${FID_TARGET} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/fidelius/parser-modules"
    INSTALL_RPATH "\$ORIGIN/../..")
  set_property(TARGET ${FID_TARGET} APPEND_STRING PROPERTY LINK_FLAGS
    " -Wl,-z,defs -Wl,--disable-new-dtags -Wl,--version-script=${FID_PARSER_PLUGIN_VERSION_SCRIPT}")

  string(REGEX REPLACE "[^A-Za-z0-9_.-]" "_" module_file_stem
         "${FID_MODULE_ID}")
  set(build_registry_dir "${CMAKE_BINARY_DIR}/fidelius/parser-registry.d")
  set(build_fragment
      "${build_registry_dir}/${module_file_stem}.module.json")
  _fid_parser_library_filename(${FID_TARGET} plugin_filename)
  add_custom_command(OUTPUT "${build_fragment}"
    COMMAND "${CMAKE_COMMAND}"
      "-DFID_REGISTRY_KIND=MODULE"
      "-DFID_REGISTRY_OUTPUT=${build_fragment}"
      "-DMODULE_ID=${FID_MODULE_ID}"
      "-DEDL_FINGERPRINT=${edl_fingerprint}"
      "-DPLUGIN_FILE=$<TARGET_FILE:${FID_TARGET}>"
      "-DRELATIVE_LIBRARY_PATH=../parser-modules/$<TARGET_FILE_NAME:${FID_TARGET}>"
      -P "${FID_PARSER_REGISTRY_GENERATOR}"
    DEPENDS ${FID_TARGET} "${FID_PARSER_REGISTRY_GENERATOR}"
      ${edl_fingerprint_DEPENDENCIES}
    VERBATIM)
  add_custom_target(${module_file_stem}-module-registry ALL
    DEPENDS "${build_fragment}")

  if(NOT CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR lib)
  endif()
  install(TARGETS ${FID_TARGET}
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/fidelius/parser-modules")

  set(install_fragment_rel
      "share/fidelius/parser-registry.d/${module_file_stem}.module.json")
  set(installed_plugin_rel
      "${CMAKE_INSTALL_LIBDIR}/fidelius/parser-modules/${plugin_filename}")
  set(installed_library_relative
      "../../../${installed_plugin_rel}")
  string(CONCAT install_registry_code
    "execute_process(\n"
    "  COMMAND \"${CMAKE_COMMAND}\"\n"
    "    \"-DFID_REGISTRY_KIND=MODULE\"\n"
    "    \"-DFID_REGISTRY_OUTPUT=\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${install_fragment_rel}\"\n"
    "    \"-DMODULE_ID=${FID_MODULE_ID}\"\n"
    "    \"-DEDL_FINGERPRINT=${edl_fingerprint}\"\n"
    "    \"-DPLUGIN_FILE=\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${installed_plugin_rel}\"\n"
    "    \"-DRELATIVE_LIBRARY_PATH=${installed_library_relative}\"\n"
    "    -P \"${FID_PARSER_REGISTRY_GENERATOR}\"\n"
    "  RESULT_VARIABLE fid_registry_result)\n"
    "if(NOT fid_registry_result EQUAL 0)\n"
    "  message(FATAL_ERROR \"Failed to generate installed parser registry fragment\")\n"
    "endif()\n")
  install(CODE "${install_registry_code}")

  set_target_properties(${FID_TARGET} PROPERTIES
    FID_PARSER_MODULE_ID "${FID_MODULE_ID}"
    FID_PARSER_EDL_FINGERPRINT "${edl_fingerprint}"
    FID_PARSER_BUILD_REGISTRY_FRAGMENT "${build_fragment}")
endfunction()

function(fid_register_parser_enclave)
  set(options EXCLUDE_FROM_ALL OPTIONAL_INSTALL)
  set(one_value NAME MODULE_ID ENCLAVE EDL)
  set(multi_value DEPENDS EDL_SEARCH_PATHS)
  cmake_parse_arguments(FID "${options}" "${one_value}" "${multi_value}" ${ARGN})
  foreach(required NAME MODULE_ID ENCLAVE EDL)
    if(NOT FID_${required})
      message(FATAL_ERROR "fid_register_parser_enclave: ${required} is required")
    endif()
  endforeach()
  if(NOT EXISTS "${FID_EDL}")
    message(FATAL_ERROR
      "fid_register_parser_enclave: EDL not found: ${FID_EDL}")
  endif()
  if(NOT SGX_ENCLAVE_SIGNER)
    message(FATAL_ERROR "fid_register_parser_enclave: SGX signer unavailable")
  endif()

  fid_compute_edl_fingerprint(
    edl_fingerprint "${FID_EDL}" ${FID_EDL_SEARCH_PATHS})
  string(REGEX REPLACE "[^A-Za-z0-9_.-]" "_" fragment_stem "${FID_NAME}")
  set(registry_dir "${CMAKE_BINARY_DIR}/fidelius/parser-registry.d")
  set(fragment "${registry_dir}/${fragment_stem}.enclave.json")
  get_filename_component(enclave_name "${FID_ENCLAVE}" NAME)

  add_custom_command(OUTPUT "${fragment}"
    COMMAND "${CMAKE_COMMAND}"
      "-DFID_REGISTRY_KIND=ENCLAVE"
      "-DFID_REGISTRY_OUTPUT=${fragment}"
      "-DMODULE_ID=${FID_MODULE_ID}"
      "-DEDL_FINGERPRINT=${edl_fingerprint}"
      "-DENCLAVE_FILE=${FID_ENCLAVE}"
      "-DENCLAVE_NAME=${enclave_name}"
      "-DSGX_SIGNER=${SGX_ENCLAVE_SIGNER}"
      -P "${FID_PARSER_REGISTRY_GENERATOR}"
    DEPENDS "${FID_ENCLAVE}" ${FID_DEPENDS}
      ${edl_fingerprint_DEPENDENCIES}
    VERBATIM)
  if(FID_EXCLUDE_FROM_ALL)
    add_custom_target(${fragment_stem}-parser-registry
      DEPENDS "${fragment}")
  else()
    add_custom_target(${fragment_stem}-parser-registry ALL
      DEPENDS "${fragment}")
  endif()
  if(FID_OPTIONAL_INSTALL)
    install(FILES "${fragment}"
      DESTINATION "share/fidelius/parser-registry.d" OPTIONAL)
  else()
    install(FILES "${fragment}"
      DESTINATION "share/fidelius/parser-registry.d")
  endif()
endfunction()
