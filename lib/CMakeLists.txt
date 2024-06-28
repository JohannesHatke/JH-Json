#cmake_minimum_required(VERSION 3.10)
#project(JH_Json)

add_library(JH_Json SHARED JH_Json.c)

add_subdirectory(UTF8)
add_subdirectory(HashTable)
add_subdirectory(ArrayList)

target_link_libraries(JH_Json Utf8 HashTable ArrayList)
