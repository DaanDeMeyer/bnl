cddm_add_library(h3c C 11)

target_sources(h3c PRIVATE
  src/h3c/frame.c
  src/h3c/varint.c
)

include(TestBigEndian)
test_big_endian(IS_BIG_ENDIAN)

target_compile_definitions(h3c PRIVATE
  $<IF:${IS_BIG_ENDIAN},H3C_BIG_ENDIAN,H3C_LITTLE_ENDIAN>
)

if(H3C_TESTS)
  target_link_libraries(h3c PRIVATE doctest)
  cddm_add_common(h3c CXX 11 lib)

  target_link_libraries(h3c-tests PRIVATE h3c)
endif()