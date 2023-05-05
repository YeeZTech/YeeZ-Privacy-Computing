set(TRAIN_SRCS train_parser.cpp)

#add_ypc_applet(train_parser
  #CRYPTO stdeth
  #SRCS ${T_SRCS})

add_enclave_library(train_parser SRCS ${TRAIN_SRCS}
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/eparser.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox/
)
target_include_directories(train_parser-edlobj PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_include_directories(train_parser PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
#target_compile_options(train_parser PRIVATE -fopenmp)
target_link_libraries(train_parser PRIVATE
  stbox_common_t
  stbox_channel_t
  analyzer_t
  stdeth_t
  sgx_omp
  sgx_pthread
  "-Wl,--gc-sections"
  "-fopenmp"
)

enclave_sign(train_parser KEY enclave_private.pem
  CONFIG enclave.config.xml)


set(PRED_SRCS pred_parser.cpp)
add_enclave_library(pred_parser SRCS ${PRED_SRCS}
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/eparser.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox/
)
target_include_directories(pred_parser-edlobj PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_include_directories(pred_parser PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(pred_parser PRIVATE
  stbox_common_t
  stbox_channel_t
  analyzer_t
  stdeth_t
  sgx_omp
  sgx_pthread
  "-Wl,--gc-sections"
  "-fopenmp"
)
enclave_sign(pred_parser KEY enclave_private.pem
  CONFIG enclave.config.xml)
