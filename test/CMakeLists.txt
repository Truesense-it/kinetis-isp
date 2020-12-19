# Copyright (c) 2020 Albert Krenz
# 
# This code is licensed under BSD + Patent (see LICENSE.txt for full license text)

# SPDX-License-Identifier: BSD-2-Clause-Patent
find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

include(GoogleTest)


add_executable(utests k32w061_test.cpp firmware_reader_test.cpp ${CMAKE_SOURCE_DIR}/src/k32w061.cpp ${CMAKE_SOURCE_DIR}/src/firmware_reader.cpp)
target_include_directories(utests PRIVATE ${CMAKE_SOURCE_DIR}/src ${GTEST_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

if(COVERAGE)
  target_compile_options(utests PRIVATE "--coverage")
  set(GCOV_LIBRARIES PRIVATE gcov)

  add_custom_target(coverage
    COMMAND gcovr -j4 -r ${CMAKE_SOURCE_DIR} -s ${CMAKE_BINARY_DIR}
  )
endif()

target_compile_options(utests PRIVATE -Wno-error=unused-parameter -Wall -Werror -Wextra)
target_link_libraries(utests PRIVATE gmock ${GTEST_LIBRARIES} ${GMOCK_BOTH_LIBRARIES} ${GCOV_LIBRARIES} ${Boost_LIBRARIES} Threads::Threads)

gtest_discover_tests(utests
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})