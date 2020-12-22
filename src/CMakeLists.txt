# Copyright (c) 2020 Albert Krenz
# 
# This code is licensed under BSD + Patent (see LICENSE.txt for full license text)

# SPDX-License-Identifier: BSD-2-Clause-Patent
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(LibFTDI1 NO_MODULE REQUIRED)

add_executable(${PROJECT_NAME} main.cpp ftdi_linux.cpp application.cpp k32w061.cpp firmware_reader.cpp vid_pid_reader.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION 0.1.0)
target_link_libraries(${PROJECT_NAME} ${LIBFTDI_LIBRARIES} ${Boost_LIBRARIES} Threads::Threads)
get_target_property(VERSION ${PROJECT_NAME} VERSION)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${LIBFTDI_DEFINITIONS} -DVERSION=\"${VERSION}\")
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBFTDI_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-error=unused-parameter -Wall -Werror -Wextra $<$<CONFIG:DEBUG>:-O0 -g3>)

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin)