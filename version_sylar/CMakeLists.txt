cmake_minimum_required(VERSION 3.0)
project(sylar)

#设置相对路径而不是绝对路径
include (cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON) # 打开cmake的编译过程
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")
# set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(.)
#include_directories(/app/sylar/include)
#link_directories(/app/sylar/lib)

find_library(YAMLCPP yaml-cpp)
#message("***", ${YAMLCPP})

set(LIB_SRC
    sylar/uri.rl.cc
    sylar/http/servlet.cc
    sylar/http/http_session.cc
    sylar/http/http_server.cc
    sylar/streams/socket_stream.cc
    sylar/stream.cc
    sylar/tcp_server.cc
    sylar/http/http_parser.cc
    sylar/http/http11_parser.rl.cc
    sylar/http/httpclient_parser.rl.cc
    sylar/http/http.cc
    sylar/bytearray.cc
    sylar/socket.cc
    sylar/address.cc
    sylar/log.cc
    sylar/util.cc
    sylar/config.cc
    sylar/mutex.cc
    sylar/thread.cc
    sylar/fiber.cc
    sylar/scheduler.cc
    sylar/iomanager.cc
    sylar/timer.cc
    sylar/hook.cc
    sylar/fd_manager.cc
    )

add_library(sylar SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(sylar) ##重定义__FILE__为相对路径
#add_library(sylar_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES (sylar_static PROPERTIES OUTPUT_NAME "sylar")

set(LIBS
    sylar
    pthread
    dl
    ${YAMLCPP}
  )

add_executable(test tests/test.cc)
add_dependencies(test sylar)
force_redefine_file_macro_for_sources(test) ##重定义__FILE__为相对路径
#target_link_libraries(test sylar yaml-cpp)
target_link_libraries(test ${LIBS})

add_executable(test_config tests/test_config.cc)
add_dependencies(test_config sylar)
force_redefine_file_macro_for_sources(test_config)
target_link_libraries(test_config sylar yaml-cpp)

add_executable(test_thread tests/test_thread.cc)
add_dependencies(test_thread sylar)
force_redefine_file_macro_for_sources(test_thread)
target_link_libraries(test_thread ${LIBS})

add_executable(test_util tests/test_util.cc)
add_dependencies(test_util sylar)
force_redefine_file_macro_for_sources(test_util)
target_link_libraries(test_util ${LIBS})

add_executable(test_fiber tests/test_fiber.cc)
add_dependencies(test_fiber sylar)
force_redefine_file_macro_for_sources(test_fiber)
target_link_libraries(test_fiber ${LIBS})

add_executable(test_scheduler tests/test_scheduler.cc)
add_dependencies(test_scheduler sylar)
force_redefine_file_macro_for_sources(test_scheduler)
target_link_libraries(test_scheduler ${LIBS})

add_executable(test_iomanager tests/test_iomanager.cc)
add_dependencies(test_iomanager sylar)
force_redefine_file_macro_for_sources(test_iomanager)
target_link_libraries(test_iomanager ${LIBS})

add_executable(test_hook tests/test_hook.cc)
add_dependencies(test_hook sylar)
force_redefine_file_macro_for_sources(test_hook)
target_link_libraries(test_hook ${LIBS})

add_executable(test_address tests/test_address.cc)
add_dependencies(test_address sylar)
force_redefine_file_macro_for_sources(test_address)
target_link_libraries(test_address ${LIBS})

add_executable(test_socket tests/test_socket.cc)
add_dependencies(test_socket sylar)
force_redefine_file_macro_for_sources(test_socket)
target_link_libraries(test_socket ${LIBS})

add_executable(test_bytearray tests/test_bytearray.cc)
add_dependencies(test_bytearray sylar)
force_redefine_file_macro_for_sources(test_bytearray)
target_link_libraries(test_bytearray ${LIBS})

add_executable(test_http tests/test_http.cc)
add_dependencies(test_http sylar)
force_redefine_file_macro_for_sources(test_http)
target_link_libraries(test_http ${LIBS})

add_executable(test_http_parser tests/test_http_parser.cc)
add_dependencies(test_http_parser sylar)
force_redefine_file_macro_for_sources(test_http_parser)
target_link_libraries(test_http_parser ${LIBS})

add_executable(test_tcp_server tests/test_tcp_server.cc)
add_dependencies(test_tcp_server sylar)
force_redefine_file_macro_for_sources(test_tcp_server)
target_link_libraries(test_tcp_server ${LIBS})

add_executable(echo_server examples/echo_server.cc)
add_dependencies(echo_server sylar)
force_redefine_file_macro_for_sources(echo_server)
target_link_libraries(echo_server ${LIBS})

add_executable(test_http_server tests/test_http_server.cc)
add_dependencies(test_http_server sylar)
force_redefine_file_macro_for_sources(test_http_server)
target_link_libraries(test_http_server ${LIBS})

add_executable(test_uri tests/test_uri.cc)
add_dependencies(test_uri sylar)
force_redefine_file_macro_for_sources(test_uri)
target_link_libraries(test_uri ${LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
