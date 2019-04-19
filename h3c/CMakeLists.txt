cddm_add_library(h3c C 11)

target_include_directories(h3c PRIVATE src)
target_sources(h3c PRIVATE
  src/frame.c
  src/varint.c
)

if(H3C_TESTS)
  target_link_libraries(h3c PRIVATE doctest)
  cddm_add_common(h3c CXX 11 lib)

  target_link_libraries(h3c-tests PRIVATE h3c)
endif()