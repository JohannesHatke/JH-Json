add_library(HashTable ./HashTable.c)

target_include_directories(HashTable INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(HashTable m)
