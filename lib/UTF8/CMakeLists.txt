add_library(Utf8 ./utf8.c) 

target_include_directories(Utf8 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
