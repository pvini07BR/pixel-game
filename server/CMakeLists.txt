cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 99)

project(server)

file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_executable(server)

set_property(TARGET server PROPERTY C_STANDARD 99)

target_sources(server PRIVATE ${SERVER_SOURCES})

target_include_directories(server PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

target_link_libraries(server PRIVATE enet_static )

if (WIN32)
	target_link_libraries(server PRIVATE pthreads-win32)
endif()