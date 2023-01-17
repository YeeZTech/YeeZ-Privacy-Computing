function(Doxygen input)
  find_package(Doxygen)
  if (NOT DOXYGEN_FOUND)
    add_custom_target(doxygen COMMAND false
      COMMENT "Doxygen not found")
    return()
  endif()
  set(DOXYGEN_GENERATE_HTML YES)
  set(DOXYGEN_EXCLUDE_PATTERNS
    */.git/*
    */.svn/*
    */.hg/*
    */CMakeFiles/*
    */_CPack_Packages/*
    DartConfiguration.tcl
    CMakeLists.txt
    CMakeCache.txt
    */test/*
    */vendor/*
    */build_debug/*
    */stbox/*)
  doxygen_add_docs(doxygen
    ${input}
    COMMENT "Generate HTML documentation")
endfunction()

