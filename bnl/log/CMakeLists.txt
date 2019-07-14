bnl_add_library(log)

target_link_libraries(bnl-log PUBLIC bnl-base)

target_sources(bnl-log PRIVATE src/console.cpp)

if(BNL_TEST)
  target_link_libraries(bnl-test PRIVATE bnl-log)
endif()
