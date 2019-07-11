bnl_add_library(util)

target_link_libraries(bnl-util PUBLIC bnl-base)

target_sources(bnl-util PRIVATE src/string.cpp)

target_link_libraries(bnl-test PRIVATE bnl-util)
