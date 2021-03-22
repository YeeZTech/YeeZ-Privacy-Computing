add_subdirectory(ypcd)
add_subdirectory(shuttle)

set(ypc_dep_libs cryptopp boost_program_options boost_filesystem boost_system glog)
add_subdirectory(datahub)
add_subdirectory(analyzer)
add_subdirectory(yprepare)
