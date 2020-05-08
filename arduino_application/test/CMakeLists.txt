cmake_minimum_required(VERSION 3.0)
project (testexample) 

enable_testing()
add_definitions(-DTEST)

include_directories(../src)
include_directories(../include)

add_library(test_main OBJECT test_main.cpp ../src/main.c ../src/spi_func.c)

add_executable(test1 joystick.test.cpp $<TARGET_OBJECTS:test_main>)
add_executable(test_response command.test.cpp $<TARGET_OBJECTS:test_main> command.test.cpp)

add_test(NAME test1 COMMAND test1)
add_test(NAME test_response COMMAND test_response)


