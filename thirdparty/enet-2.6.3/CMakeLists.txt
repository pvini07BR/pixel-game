cmake_minimum_required(VERSION 3.10)
project(enet C)

# -----------------------------
# 1) Build options
# -----------------------------
option(ENET_STATIC "Build enet as a static library" ON)
option(ENET_SHARED "Build enet as a shared library" OFF)
option(ENET_TEST  "Build enet tests" OFF)
option(ENET_USE_MORE_PEERS  "Build enet with up to 65k peers support" OFF)

if (ENET_USE_MORE_PEERS)
    add_definitions(-DENET_USE_MORE_PEERS)
endif()

# -----------------------------
# 2) Static library
# -----------------------------
if(ENET_STATIC)
    add_library(enet_static STATIC test/library.c)

    if(WIN32)
        target_link_libraries(enet_static PUBLIC winmm ws2_32)
        if(MSVC)
            target_compile_options(enet_static PRIVATE /W3)
        endif()
    else()
        target_compile_options(enet_static PRIVATE -Wno-error)
    endif()

    target_include_directories(enet_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

    add_library(enet::enet_static ALIAS enet_static)
endif()

# -----------------------------
# 3) Shared library
# -----------------------------
if(ENET_SHARED)
    add_library(enet_shared SHARED test/library.c)
    target_compile_definitions(enet_shared PUBLIC -DENET_DLL)

    if(WIN32)
        target_link_libraries(enet_shared PUBLIC winmm ws2_32)
        if(MSVC)
            target_compile_options(enet_shared PRIVATE /W3)
        endif()
    else()
        target_compile_options(enet_shared PRIVATE -Wno-error)
    endif()

    target_include_directories(enet_shared PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

    add_library(enet::enet_shared ALIAS enet_shared)
endif()

# -----------------------------
# 4) Tests (optional)
# -----------------------------
if(ENET_TEST)
    add_library(enet_test_interface INTERFACE)

    target_include_directories(enet_test_interface INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    if(TARGET enet_static)
        target_link_libraries(enet_test_interface INTERFACE enet_static)
    elseif(TARGET enet_shared)
        target_link_libraries(enet_test_interface INTERFACE enet_shared)
    endif()

    if(WIN32)
        target_link_libraries(enet_test_interface INTERFACE winmm ws2_32)
    endif()

    # Default test
    add_executable(enet_test test/build.c)
    target_link_libraries(enet_test PRIVATE enet_test_interface)
    
    # Test with more peers
    add_executable(enet_test_extra_peers test/build.c)
    target_link_libraries(enet_test_extra_peers PRIVATE enet_test_interface)
    target_compile_definitions(enet_test_extra_peers PRIVATE ENET_USE_MORE_PEERS)

endif()

# -----------------------------
# 5) Installation
# -----------------------------
set(_enet_targets_to_install "")

if(TARGET enet_static)
    list(APPEND _enet_targets_to_install enet_static)
    set_target_properties(enet_static PROPERTIES OUTPUT_NAME "enet")
endif()

if(TARGET enet_shared)
    list(APPEND _enet_targets_to_install enet_shared)
    set_target_properties(enet_shared PROPERTIES OUTPUT_NAME "enet")
endif()

if(_enet_targets_to_install)
    install(
        TARGETS ${_enet_targets_to_install}
        EXPORT enetTargets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include
    )

    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include
        FILES_MATCHING
        PATTERN "*.h"
    )

    install(
        EXPORT enetTargets
        FILE enetConfig.cmake
        NAMESPACE enet::
        DESTINATION lib/cmake/enet
    )
endif()

