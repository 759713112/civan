cmake_minimum_required(VERSION 3.0)
project(civan)

include (cmake/utils.cmake)


#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdpart/yaml-cpp)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdpart/jsoncpp-1.8.4)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdpart/sqlite-3.28.0)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdpart/tinyxml2-7.0.1)



set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")


include_directories(.)
include_directories(/apps/civan/include)
link_directories(/apps/civan/lib)

set(LIB_SRC
    civan/config.cc
    civan/fiber.cc
    civan/log.cc
    civan/thread.cc
    civan/util.cc
    civan/scheduler.cc
    civan/iomanager.cc
    civan/timer.cc
    civan/hook.cc
    civan/fd_manager.cc
    civan/address.cc
    civan/socket.cc
    civan/bytearray.cc
    civan/http/http.cc
    civan/http/http_parser.cc
    # civan/http/http_connection.cc
    civan/http/http_session.cc
    civan/http/http_server.cc
    civan/http/servlet.cc
    civan/tcp_server.cc
    civan/stream.cc
    civan/socket_stream.cc


    )

ragelmaker(civan/http/http11_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/civan/http)
ragelmaker(civan/http/httpclient_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/civan/http)
ragelmaker(civan/uri.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/civan)

add_library(civan SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(civan) #__FILE__
#add_library(civan_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES (civan_static PROPERTIES OUTPUT_NAME "civan")

set (LIB_LIB
    civan
    pthread
    yaml-cpp
    dl
    )


find_library(YAMLCPP yaml-cpp)
find_library(PTHREAD pthread)
#set(LIBS ${LIBS} civan)
#set(LIBS ${LIBS} dl)
#set(LIBS ${LIBS} ${YAMLCPP})
#set(LIBS ${LIBS} ${PTHREAD})




add_executable(test_config "tests/test_config.cc")
add_dependencies(test_config civan)
force_redefine_file_macro_for_sources(test_config)  #__FILE__
target_link_libraries(test_config ${LIB_LIB})

add_executable(test2 "tests/test.cc")
add_dependencies(test2 civan)
force_redefine_file_macro_for_sources(test2)  #__FILE__
target_link_libraries(test2 ${LIB_LIB})

add_executable(test_thread "tests/test_thread.cc")
add_dependencies(test_thread civan)
force_redefine_file_macro_for_sources(test_thread)  #__FILE__
target_link_libraries(test_thread ${LIB_LIB})


add_executable(test_util "tests/test_util.cc")
add_dependencies(test_util civan)
force_redefine_file_macro_for_sources(test_util)  #__FILE__
target_link_libraries(test_util ${LIB_LIB})

add_executable(test_fiber "tests/test_fiber.cc")
add_dependencies(test_fiber civan)
force_redefine_file_macro_for_sources(test_fiber)  #__FILE__
target_link_libraries(test_fiber ${LIB_LIB})

add_executable(test_scheduler "tests/test_scheduler.cc")
add_dependencies(test_scheduler civan)
force_redefine_file_macro_for_sources(test_scheduler)  #__FILE__
target_link_libraries(test_scheduler ${LIB_LIB})

add_executable(test_iomanager "tests/test_iomanager.cc")
add_dependencies(test_iomanager civan)
force_redefine_file_macro_for_sources(test_iomanager)  #__FILE__
target_link_libraries(test_iomanager ${LIB_LIB})


add_executable(test_epoll_wait "tests/test_epoll_wait.cc")
add_dependencies(test_epoll_wait civan)
force_redefine_file_macro_for_sources(test_epoll_wait)  #__FILE__
target_link_libraries(test_epoll_wait ${LIB_LIB})


add_executable(test_hook "tests/test_hook.cc")
add_dependencies(test_hook civan)
force_redefine_file_macro_for_sources(test_hook)  #__FILE__
target_link_libraries(test_hook ${LIB_LIB})

add_executable(test_address "tests/test_address.cc")
add_dependencies(test_address civan)
force_redefine_file_macro_for_sources(test_address)  #__FILE__
target_link_libraries(test_address ${LIB_LIB})

add_executable(test_socket "tests/test_socket.cc")
add_dependencies(test_socket civan)
force_redefine_file_macro_for_sources(test_socket)  #__FILE__
target_link_libraries(test_socket ${LIB_LIB})

add_executable(test_bytearray "tests/test_bytearray.cc")
add_dependencies(test_bytearray civan)
force_redefine_file_macro_for_sources(test_bytearray)  #__FILE__
target_link_libraries(test_bytearray ${LIB_LIB})

# add_executable(test_http_parser "tests/test_http_parser.cc")
# add_dependencies(test_http_parser civan)
# force_redefine_file_macro_for_sources(test_http_parser)  #__FILE__
# target_link_libraries(test_http_parser ${LIB_LIB})

# add_executable(echo_server "examples/echo_server.cc")
# add_dependencies(echo_server civan)
# force_redefine_file_macro_for_sources(echo_server)  #__FILE__
# target_link_libraries(echo_server ${LIB_LIB})


# add_executable(test_http_server "tests/test_http_server.cc")
# add_dependencies(test_http_server civan)
# force_redefine_file_macro_for_sources(test_http_server)  #__FILE__
# target_link_libraries(test_http_server ${LIB_LIB})


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)