set(T_SRCS eparser.cpp)

#add_ypc_applet(person_first_match
  #CRYPTO stdeth
  #SRCS ${T_SRCS})

add_enclave_library(person_first_match SRCS ${T_SRCS}
  EDL personlist.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/edl/util/:${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox/
)
target_include_directories(person_first_match-edlobj PUBLIC
  "${PROJECT_SOURCE_DIR}/include"
  )
target_include_directories(person_first_match PUBLIC
  "${PROJECT_SOURCE_DIR}/include"
  )
target_link_libraries(person_first_match PRIVATE
  stbox_common_t
  stbox_channel_t
  analyzer_t stdeth_t)

if(SGX_MODE STREQUAL "Debug")
  enclave_sign(person_first_match KEY enclave_private.pem
    CONFIG enclave.config.debug.xml)
else()
  enclave_sign(person_first_match KEY enclave_private.pem
    CONFIG enclave.config.xml)
endif()
