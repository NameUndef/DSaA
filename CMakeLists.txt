cmake_minimum_required(VERSION 3.19)
project(DSAA)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

add_executable(${PROJECT_NAME} main.c)
add_library(RetCode STATIC ret_code.c)

add_library(RingBuffer STATIC ring_buffer.c ring_buffer_demo.c)
target_link_libraries(RingBuffer PRIVATE RetCode)

add_library(PortalQueue STATIC 
    portal_queue.c 
    portal_queue_demo.c)
target_link_libraries(PortalQueue PRIVATE RetCode)

add_library(ArenaAllocator STATIC
    arena_allocator.c
    arena_allocator_demo.c)
target_link_libraries(ArenaAllocator PRIVATE RetCode)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    RingBuffer 
    PortalQueue 
    ArenaAllocator)

install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug RELEASE DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
