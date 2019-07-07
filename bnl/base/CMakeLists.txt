bnl_add_library(base)

target_link_libraries(bnl-base PUBLIC fmt)

target_sources(bnl-base PRIVATE
  src/http3/event.cpp
  src/http3/header.cpp
  src/quic/event.cpp
  src/buffer_view.cpp
  src/buffer.cpp
  src/buffers.cpp
  src/error.cpp
  src/log.cpp
  src/string_view.cpp
)

if(BNL_TEST)
  target_sources(bnl-test PRIVATE
    test/buffer.cpp
    test/buffers.cpp
  )

  target_link_libraries(bnl-test PRIVATE bnl-base)
endif()
