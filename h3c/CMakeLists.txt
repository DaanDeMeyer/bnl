cddm_add_library(h3c C 11)

target_include_directories(h3c PRIVATE src)
target_sources(h3c PRIVATE
  src/error.c
  src/frame/decode.c
  src/frame/encode.c
  src/settings.c
  src/varint/decode.c
  src/varint/encode.c
)

if(H3C_TEST)
  target_sources(h3c-test PRIVATE
    test/frame.cpp
    test/varint.cpp
  )

  target_link_libraries(h3c-test PRIVATE h3c)
endif()