cmake_minimum_required (VERSION 3.3)
project (metl CXX)

file(GLOB SOURCES *.h)

list(APPEND SOURCES
	../metl.h
)

# target for "linking" to
add_library(metl INTERFACE)
target_include_directories(metl INTERFACE "${PEGTL_INCLUDE_DIR}")
target_include_directories(metl INTERFACE "${METL_ROOT_DIR}/include")

# target for showing headers in IDE
add_custom_target(metl_impl SOURCES ${SOURCES})
