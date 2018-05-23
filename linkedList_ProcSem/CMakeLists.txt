cmake_minimum_required(VERSION 3.10)
project(linkedList_sem)

set(CMAKE_CXX_STANDARD 11)

add_executable(linkedList_sem LinkedList.c)

target_link_libraries(linkedList_sem pthread)
