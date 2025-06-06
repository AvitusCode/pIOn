cmake_minimum_required(VERSION 3.17)

project(pIOn VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ROOT_DIR "${CMAKE_SOURCE_DIR}/")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

#set(BOOST_ROOT "C:/Program Files/boost/boost_1_82_0/")
#set(Boost_NO_SYSTEM_PATHS ON)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_LIBS ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
#set(BOOST_ALL_DYN_LINK OFF)

#find_package(Boost 1.82.0 REQUIRED COMPONENTS intrusive REQUIRED)

if (Boost_FOUND)
message(CHECK_PASS "Boost found")
else()
message(CHECK_FAIL "Boost missed")
endif()

#include_directories(SYSTEM ${Boost_INCLUDE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

add_subdirectory(sequitor)

add_executable(pIOn main.cpp 
                    src/blktrace_parcer.cpp 
                    src/research/research.cpp 
                    src/research/config.cpp)
set_property(TARGET pIOn PROPERTY CXX_STANDARD 20)
target_link_libraries(pIOn PRIVATE jdSequitor nlohmann_json::nlohmann_json)

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