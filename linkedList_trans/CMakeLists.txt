cmake_minimum_required(VERSION 3.10)
project(linkedList_trans)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_FLAGS "-fgnu-tm")

add_executable(linkedList_trans LinkedList.c)

target_link_libraries(linkedList_trans pthread)