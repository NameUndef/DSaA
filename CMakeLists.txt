cmake_minimum_required(VERSION 3.19)
project(DSAA)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

add_executable(${PROJECT_NAME} main.c)
add_library(RetCode STATIC ret_code.c)

add_library(PageManage STATIC page_manage.c)

add_library(Alignment STATIC alignment.c)

add_library(MemDemoMap STATIC mem_demo_map.c)

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

add_library(PoolAllocator STATIC
    pool_allocator.c
    pool_allocator_demo.c)
target_link_libraries(PoolAllocator PRIVATE 
    RetCode
    Alignment
    PageManage
    MemDemoMap)

add_library(PageAllocator STATIC
    page_allocator.c
    page_allocator_demo.c)
target_link_libraries(PageAllocator PRIVATE 
    RetCode
    PageManage
    Alignment
    MemDemoMap)

add_library(SlabAllocator STATIC
    slab_allocator.c
    slab_allocator_demo.c)
target_link_libraries(SlabAllocator PRIVATE 
    RetCode
    PageManage
    PageAllocator
    Alignment
    MemDemoMap)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    RingBuffer 
    PortalQueue 
    ArenaAllocator
    PoolAllocator
    PageAllocator
    SlabAllocator)

install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug RELEASE DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
