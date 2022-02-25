
pybind11_add_module(pyterminus module.cpp)
target_link_libraries(pyterminus PRIVATE boost_system ypc_terminus glog ypc_core)
