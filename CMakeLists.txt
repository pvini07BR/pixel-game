cmake_minimum_required(VERSION 3.13)

project(pixel-game)

add_subdirectory(thirdparty/raylib-5.5)
add_subdirectory(thirdparty/imgui-1.91.9b)
add_subdirectory(thirdparty/rlImGui)
add_subdirectory(thirdparty/enet-2.6.3)
if (WIN32)
	add_subdirectory(thirdparty/pthreads-win32)
endif()

add_subdirectory(client)
add_subdirectory(server)
