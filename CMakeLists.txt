cmake_minimum_required (VERSION 3.12)

project(pIOn VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ROOT_DIR "${CMAKE_SOURCE_DIR}/")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

add_subdirectory(sequitor)

set(TESTS_FILES tests/tests.cpp)

add_executable(pIOn main.cpp ${TESTS_FILES})
set_property(TARGET pIOn PROPERTY CXX_STANDARD 17)
target_link_libraries(pIOn PRIVATE jdSequitor)
