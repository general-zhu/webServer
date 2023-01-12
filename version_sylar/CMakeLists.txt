cmake_minimum_required(VERSION 3.0)
project(sylar)

#设置相对路径而不是绝对路径
include (cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON) # 打开cmake的编译过程
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(.)
#include_directories(/app/sylar/include)
#link_directories(/app/sylar/lib)

#find_library(YAMLCPP yaml-cpp)
#message("***", ${YAMLCPP})

set(LIB_SRC
    sylar/log.cc
    sylar/util.cc
    sylar/config.cc
    )

add_library(sylar SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(sylar) ##重定义__FILE__为相对路径
#add_library(sylar_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES (sylar_static PROPERTIES OUTPUT_NAME "sylar")

add_executable(test tests/test.cc)
add_dependencies(test sylar)
force_redefine_file_macro_for_sources(test) ##重定义__FILE__为相对路径
target_link_libraries(test sylar yaml-cpp)

add_executable(test_config tests/test_config.cc)
add_dependencies(test_config sylar)
force_redefine_file_macro_for_sources(test_config) ##重定义__FILE__为相对路径
target_link_libraries(test_config sylar yaml-cpp)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
