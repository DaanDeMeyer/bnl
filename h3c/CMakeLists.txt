cddm_add_library(h3c C 11)

target_link_libraries(h3c PRIVATE xxhash)
target_include_directories(h3c PRIVATE src)
target_sources(h3c PRIVATE
  src/frame/decode.c
  src/frame/encode.c
  src/huffman/encode.c
  src/huffman/decode.c
  src/qpack/encode.c
  src/qpack/decode.c
  src/util/string.c
  src/varint/decode.c
  src/varint/encode.c
  src/error.c
  src/settings.c
)

# Make h3c-log available so we can use it in the tests.
add_subdirectory(../h3c-log ../h3c-log)

if(H3C_TEST)
  target_sources(h3c-test PRIVATE
    test/frame.cpp
    test/huffman.cpp
    test/qpack.cpp
    test/varint.cpp
  )

  target_link_libraries(h3c-test PRIVATE h3c h3c-log)
endif()

add_executable(h3c-qpack-encode)

cddm_add_common(h3c-qpack-encode CXX 11 bin)
target_link_libraries(h3c-qpack-encode PRIVATE h3c h3c-log)
if(MSVC)
  target_compile_definitions(h3c-qpack-encode PRIVATE CRT_SECURE_NO_WARNINGS)
endif()
target_sources(h3c-qpack-encode PRIVATE app/qpack-encode.cpp)

add_executable(h3c-qpack-decode)

cddm_add_common(h3c-qpack-decode CXX 11 bin)
target_link_libraries(h3c-qpack-decode PRIVATE h3c h3c-log)
target_sources(h3c-qpack-decode PRIVATE app/qpack-decode.cpp)
if(MSVC)
  target_compile_definitions(h3c-qpack-decode PRIVATE CRT_SECURE_NO_WARNINGS)
endif()

