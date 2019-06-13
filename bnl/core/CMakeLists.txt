cddm_add_library(core)

target_link_libraries(bnl-core PUBLIC fmt)

target_sources(bnl-core PRIVATE
  src/buffer.cpp
  src/buffers.cpp
  src/log.cpp
)

if(BNL_TEST)
  target_sources(bnl-test PRIVATE
    test/buffer.cpp
    test/buffers.cpp
  )

  target_link_libraries(bnl-test PRIVATE bnl-core)
endif()
