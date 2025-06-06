cmake_minimum_required(VERSION 3.17)

set(SEQUITOR jdSequitor)

set(SEQUITOR_SRC
        src/predictor.cpp
        src/rules.cpp
        src/symbols.cpp
        src/model/io_prophet.cpp
        src/model/blk_info.cpp
        src/key_functions/standart_key.cpp
        src/key_functions/simple_key.cpp
)

add_library(${SEQUITOR} STATIC ${SEQUITOR_SRC})
#target_link_libraries(${SEQUITOR} PUBLIC ${Boost_LIBRARIES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SEQUITOR_SRC})

target_include_directories(${SEQUITOR} PUBLIC includes)
target_include_directories(${SEQUITOR} PRIVATE src)
target_compile_features(${SEQUITOR} PUBLIC cxx_std_20)
