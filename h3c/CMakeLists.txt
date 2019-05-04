cddm_add_library(h3c C 11)

target_include_directories(h3c PRIVATE src)
target_sources(h3c PRIVATE
  src/error.c
  src/frame/parse.c
  src/frame/serialize.c
  src/settings.c
  src/varint/parse.c
  src/varint/serialize.c
)

if(H3C_TEST)
  target_sources(h3c-test PRIVATE
    test/frame.cpp
    test/varint.cpp
  )

  target_link_libraries(h3c-test PRIVATE h3c)
endif()