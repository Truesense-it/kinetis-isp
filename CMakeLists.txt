# Copyright (c) 2020 Albert Krenz
# 
# This code is licensed under BSD + Patent (see LICENSE.txt for full license text)

# SPDX-License-Identifier: BSD-2-Clause-Patent
project(nxp-isp)
cmake_minimum_required(VERSION 3.7)

option(COVERAGE "Enable creation of Unit test coverage data" OFF)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(${CMAKE_VERSION} VERSION_LESS "3.9.4") 
    message("No LTO Support in old CMake")
else()
  cmake_policy(SET CMP0069 NEW)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT LTO_SUPPORTED)
  if(LTO_SUPPORTED)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endif()

add_definitions(-DBOOST_LOG_DYN_LINK)
find_package(Boost COMPONENTS program_options log log_setup REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(src)
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()