cmake_minimum_required(VERSION 3.13)

project(pthreads-win32)

if(NOT MSVC)
	add_compile_options(-Wno-pointer-sign)
endif()

add_library(pthreads-win32)

target_sources(pthreads-win32 PRIVATE
	attr.c
	barrier.c
	cancel.c
	cleanup.c
	condvar.c
	create.c
	dll.c
	exit.c
	fork.c
	global.c
	misc.c
	mutex.c
	private.c
	rwlock.c
	sched.c
	semaphore.c
	spin.c
	sync.c
	tsd.c
	nonportable.c
)

target_compile_definitions(pthreads-win32 PUBLIC PTW32_STATIC_LIB HAVE_STRUCT_TIMESPEC PTW32_BUILD)
target_include_directories(pthreads-win32 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/")