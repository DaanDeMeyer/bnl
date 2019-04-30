#pragma once

#include <h3c/export.h>
#include <h3c/varint.h>

#include <stdint.h>

// Setting identifiers

#define H3C_SETTINGS_MAX_HEADER_LIST_SIZE 0x6U
#define H3C_SETTINGS_NUM_PLACEHOLDERS 0x9U
#define H3C_SETTINGS_QPACK_MAX_TABLE_CAPACITY 0x1U
#define H3C_SETTINGS_QPACK_BLOCKED_STREAMS 0x7U

// Setting limits

#define H3C_SETTINGS_MAX_HEADER_LIST_SIZE_MAX (H3C_VARINT_MAX)
#define H3C_SETTINGS_NUM_PLACEHOLDERS_MAX (H3C_VARINT_MAX)
#define H3C_SETTINGS_QPACK_MAX_TABLE_CAPACITY_MAX ((1U << 30) - 1)
#define H3C_SETTINGS_QPACK_BLOCKED_STREAMS_MAX ((1U << 16) - 1)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint64_t max_header_list_size;
  uint64_t num_placeholders;
  uint32_t qpack_max_table_capacity;
  uint16_t qpack_blocked_streams;
} h3c_settings_t;

H3C_EXPORT extern const h3c_settings_t h3c_settings_default;

#ifdef __cplusplus
}
#endif
