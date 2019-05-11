cddm_add_library(h3c-log C 11)

target_link_libraries(h3c-log PRIVATE h3c)
target_include_directories(h3c-log PRIVATE src)
target_compile_definitions(h3c-log PRIVATE _CRT_SECURE_NO_WARNINGS)
target_sources(h3c-log PRIVATE
  src/fprintf.c
)