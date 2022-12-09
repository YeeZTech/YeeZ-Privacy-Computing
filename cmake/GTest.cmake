
if(DEFINED ENV{GTEST_REPO})
  set(GIT_REPO $ENV{GTEST_REPO})
else()
  set(GIT_REPO "https://github.com/google/googletest.git")
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY ${GIT_REPO}
  GIT_TAG origin/main
)

#For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
option(INSTALL_GMOCK "Install GMock" OFF)
option(INSTALL_GTEST "Install GTest" ON)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

