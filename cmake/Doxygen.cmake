function(Doxygen input)
  find_package(Doxygen)
  if (NOT DOXYGEN_FOUND)
    add_custom_target(doxygen COMMAND false
      COMMENT "Doxygen not found")
    return()
  endif()
  set(DOXYGEN_GENERATE_HTML YES)
  doxygen_add_docs(doxygen
    ${input}
    COMMENT "Generate HTML documentation")
endfunction()

