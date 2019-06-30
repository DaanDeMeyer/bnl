bnl_add_library(util)

target_link_libraries(bnl-util PUBLIC bnl-core)

target_sources(bnl-util PRIVATE src/string.cpp)
