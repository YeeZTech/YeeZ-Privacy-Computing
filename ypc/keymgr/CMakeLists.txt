
set(ypc_dep_libs cryptopp boost_program_options boost_filesystem boost_system glog)

file(GLOB sub_folders ./*)
foreach(folder ${sub_folders})
  if(IS_DIRECTORY ${folder})
    add_subdirectory(${folder})
  endif()
endforeach()
