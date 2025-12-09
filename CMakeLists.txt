cmake_minimum_required(VERSION 3.19)
project(DSAA)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

add_executable(${PROJECT_NAME} main.c)
add_library(RetCode STATIC ret_code.c)

add_library(RingBuffer STATIC ring_buffer.c ring_buffer_demo.c)
target_link_libraries(RingBuffer PRIVATE RetCode)
target_link_libraries(${PROJECT_NAME} PRIVATE RingBuffer)

install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug RELEASE DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
