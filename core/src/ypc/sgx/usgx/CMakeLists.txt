add_library(ypc_usgx SHARED
  http_request.cpp)
install(TARGETS ypc_usgx DESTINATION lib COMPONENT c_ypc_core OPTIONAL)
