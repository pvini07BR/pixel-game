cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 20)

project(client)

file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(client)

set_property(TARGET client PROPERTY CXX_STANDARD 20)

target_compile_definitions(client PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

target_sources(client PRIVATE ${CLIENT_SOURCES})

target_include_directories(client PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

target_link_libraries(client PRIVATE raylib_static imgui rlimgui enet_static )
if (WIN32)
	target_link_libraries(client PRIVATE pthreads-win32)
endif()
