# Copyright (c) 2020 Albert Krenz
# 
# This code is licensed under BSD + Patent (see LICENSE.txt for full license text)

# SPDX-License-Identifier: BSD-2-Clause-Patent
set (CMAKE_CXX_STANDARD 14)

find_package ( LibFTDI1 NO_MODULE REQUIRED )
# find_package(ZLIB)

add_executable(${PROJECT_NAME} main.cpp ftdi_linux.cpp k32w061.cpp)
target_link_libraries(${PROJECT_NAME} ${LIBFTDI_LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${LIBFTDI_DEFINITIONS})
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBFTDI_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-error=unused-parameter -Wall -Werror -Wextra $<$<CONFIG:DEBUG>:-O0 -g3>)