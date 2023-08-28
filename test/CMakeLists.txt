cmake_minimum_required(VERSION 1.13)
project(TestOrderCache)

#Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(include)

#set(GTest_DIR "D:/Code/thirparty/vcpkg/installed/x86-windows/share/gtest" CACHE PATH "GTEST Library path" )
#find_package(GTest CONFIG REQUIRED)
#target_link_libraries(main PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

# Add source to this project's executable.
add_executable (TestOrderCache
	TestOrderCache.cpp)


target_link_libraries(
  TestOrderCache
  GTest::gtest_main
)

#include(GoogleTest)
