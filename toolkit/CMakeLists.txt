
set(ypc_dep_libs boost_program_options boost_filesystem boost_system glog)

file(GLOB sub_folders ./*)
foreach(folder ${sub_folders})
  if(IS_DIRECTORY ${folder})
    if(EXISTS ${folder}/CMakeLists.txt)
      add_subdirectory(${folder})
    endif()
  endif()
endforeach()
