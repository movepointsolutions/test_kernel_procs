cmake_minimum_required (VERSION 2.8)
project (test_kernel_procs)

set (SOURCES
	test_kernel_procs
)

set (HEADERS
	test_kernel_procs.h
)

include_directories ("..")

add_executable (test_kernel_procs test_kernel_procs.c)
