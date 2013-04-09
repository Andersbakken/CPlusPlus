cmake_minimum_required(VERSION 2.8)
include(${CMAKE_CURRENT_LIST_DIR}/rparser.cmake)
set(RCT_INCLUDE_DIR rct/rct)
include(${CMAKE_CURRENT_LIST_DIR}/rct/rct.cmake)
add_executable(rparser ${RCT_SOURCES} src/main.cpp ${RPARSER_SOURCES})
target_link_libraries(rparser crypto pthread rt)

