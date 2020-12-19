# Copyright (c) 2020 Albert Krenz
# 
# This code is licensed under BSD + Patent (see LICENSE.txt for full license text)

# SPDX-License-Identifier: BSD-2-Clause-Patent
project(nxp-isp)
cmake_minimum_required(VERSION 3.13)

option(COVERAGE "Enable creation of Unit test coverage data" OFF)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT error)
if(LTO_SUPPORTED)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

find_package(Boost COMPONENTS program_options log log_setup REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(src)
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()