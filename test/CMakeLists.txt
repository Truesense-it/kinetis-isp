# Copyright (c) 2020 Albert Krenz
# 
# This code is licensed under BSD + Patent (see LICENSE.txt for full license text)

# SPDX-License-Identifier: BSD-2-Clause-Patent
find_package(GTest REQUIRED)
find_package(GMock REQUIRED)
find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(GoogleTest)
# find_package(ZLIB)


add_executable(k32w061_test k32w061_test.cpp ${CMAKE_SOURCE_DIR}/src/k32w061.cpp)
target_include_directories(k32w061_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${GTEST_INCLUDE_DIRS})

if(COVERAGE)
  target_compile_options(k32w061_test PRIVATE "--coverage")
  set(GCOV_LIBRARIES PRIVATE gcov)

  add_custom_target(coverage
    COMMAND gcovr -j4 -r ${CMAKE_SOURCE_DIR} -s ${CMAKE_BINARY_DIR}
  )
endif()

target_compile_options(k32w061_test PRIVATE "-Wno-error=unused-parameter")
target_link_libraries(k32w061_test PRIVATE gmock ${GTEST_LIBRARIES} ${GMOCK_BOTH_LIBRARIES} ${GCOV_LIBRARIES} Threads::Threads)

gtest_discover_tests(k32w061_test
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})