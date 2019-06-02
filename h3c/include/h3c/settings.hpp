#pragma once

#include <h3c/export.hpp>

#include <cstdint>

namespace h3c {

namespace setting {

namespace max_header_list_size {
static constexpr uint64_t id = 0x6U;
static constexpr uint64_t max = (0x40ULL << 56U) - 1;
} // namespace max_header_list_size

namespace num_placeholders {
static constexpr uint64_t id = 0x9U;
static constexpr uint64_t max = (0x40ULL << 56U) - 1;
} // namespace num_placeholders

namespace qpack_max_table_capacity {
static constexpr uint64_t id = 0x1U;
static constexpr uint64_t max = (1U << 30U) - 1;
} // namespace qpack_max_table_capacity

namespace qpack_blocked_streams {
static constexpr uint64_t id = 0x7U;
static constexpr uint64_t max = (1U << 16U) - 1;
} // namespace qpack_blocked_streams

} // namespace setting

struct settings {
  uint64_t max_header_list_size = (0x40ULL << 56U) - 1;
  uint64_t num_placeholders = 0;
  uint32_t qpack_max_table_capacity = 0;
  uint16_t qpack_blocked_streams = 0;
};

} // namespace h3c
