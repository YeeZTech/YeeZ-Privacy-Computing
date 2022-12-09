
function(AddClangTidy target)
  find_program(CLANG-TIDY_PATH clang-tidy)
  if(NOT CLANG-TIDY_PATH)
    message(WARNING "not found clang-tidy, ignore it for " ${target})
  else()
  set_target_properties(${target}
    PROPERTIES CXX_CLANG_TIDY
    "${CLANG-TIDY_PATH};--fix;--config="
    )
  endif()
endfunction()


