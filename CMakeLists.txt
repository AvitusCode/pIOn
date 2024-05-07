cmake_minimum_required(VERSION 3.12)

project(pIOn VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ROOT_DIR "${CMAKE_SOURCE_DIR}/")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

add_subdirectory(sequitor)

add_executable(pIOn main.cpp src/blktrace_parcer.cpp src/research/research.cpp)
set_property(TARGET pIOn PROPERTY CXX_STANDARD 17)
target_link_libraries(pIOn PRIVATE jdSequitor)

target_include_directories(pIOn PUBLIC includes)
target_include_directories(pIOn PRIVATE src)

# testing binaries
add_executable(pIOn_test tests/tests.cpp tests/jd_test.cpp src/blktrace_parcer.cpp)
target_link_libraries(pIOn_test PRIVATE jdSequitor)
target_include_directories(pIOn_test PUBLIC includes)
target_include_directories(pIOn_test PRIVATE src)

# enable testing functionality
enable_testing()

#define tests
add_test(
    NAME pIOn_test
    COMMAND $<TARGET_FILE:pIOn_test>
)