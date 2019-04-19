add_library(doctest)

target_include_directories(doctest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(doctest PRIVATE impl.cpp)
