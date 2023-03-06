
pybind11_add_module(pyterminus module.cpp)
target_link_libraries(pyterminus PRIVATE boost_system terminus glog core)
install(TARGETS pyterminus
  DESTINATION "${lib_install_dir}"
  COMPONENT c_yterminus OPTIONAL)
