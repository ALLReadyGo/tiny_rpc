cmake_minimum_required(VERSION 3.5)
project(main)

find_package(Boost REQUIRED COMPONENTS)
find_package(Threads REQUIRED COMPONENTS)


add_executable(client RpcClient.cc)
add_executable(server RpcService.cc)

target_link_libraries(server ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(client ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
