
file(GLOB sub_folders ./*)
foreach(folder ${sub_folders})
  if(IS_DIRECTORY ${folder})
    if(EXISTS ${folder}/CMakeLists.txt)
      add_subdirectory(${folder})
    endif()
  endif()
endforeach()
