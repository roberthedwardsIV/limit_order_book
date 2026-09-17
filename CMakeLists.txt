cmake_minimum_required(VERSION 3.16)
project(limit_order_book LANGUAGES CXX)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_CXX_STANDARD 17)

add_executable(limit_order_book main.cpp)

include(FetchContent)
enable_testing()
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(book_tests test.cpp)

target_link_libraries(book_tests PRIVATE GTest::gtest_main)

add_executable(bench bench.cpp)
add_executable(replay replay.cpp)


