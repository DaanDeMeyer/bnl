add_library(doctest OBJECT)

target_compile_features(doctest PUBLIC cxx_std_11)
target_include_directories(doctest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(doctest PRIVATE main.cpp)