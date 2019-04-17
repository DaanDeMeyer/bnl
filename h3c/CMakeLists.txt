cddm_add_library(h3c C 11)

target_sources(h3c PRIVATE
  src/frame.c
  src/varint.c
)

include(TestBigEndian)
test_big_endian(IS_BIG_ENDIAN)

target_compile_definitions(h3c PRIVATE
  $<IF:${IS_BIG_ENDIAN},H3C_BIG_ENDIAN,H3C_LITTLE_ENDIAN>
)