set(T_SRCS t_org_info_parser.cpp)
add_ypc_applet(t_org_info_parser
  CRYPTO stdeth
  SRCS ${T_SRCS})
if(SGX_MODE STREQUAL "Debug")
  enclave_sign(t_org_info_parser KEY enclave_private.pem
    CONFIG enclave.config.debug.xml)
else()
  enclave_sign(t_org_info_parser KEY enclave_private.pem
    CONFIG enclave.config.xml)
endif()

set(T_SRCS t_tax_parser.cpp)
add_ypc_applet(t_tax_parser
  CRYPTO stdeth
  SRCS ${T_SRCS})
if(SGX_MODE STREQUAL "Debug")
  enclave_sign(t_tax_parser KEY enclave_private.pem
    CONFIG enclave.config.debug.xml)
else()
  enclave_sign(t_tax_parser KEY enclave_private.pem
    CONFIG enclave.config.xml)
endif()

set(T_SRCS merged_parser.cpp)
add_ypc_applet(t_org_tax_parser
  CRYPTO stdeth
  SRCS ${T_SRCS})
if(SGX_MODE STREQUAL "Debug")
  enclave_sign(t_org_tax_parser KEY enclave_private.pem
    CONFIG enclave.config.debug.xml)
else()
  enclave_sign(t_org_tax_parser KEY enclave_private.pem
    CONFIG enclave.config.xml)
endif()
