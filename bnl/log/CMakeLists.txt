bnl_add_library(log)

target_link_libraries(bnl-log PUBLIC bnl-core)

target_sources(bnl-log PRIVATE src/console.cpp)
