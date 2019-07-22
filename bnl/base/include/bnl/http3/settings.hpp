#pragma once

#include <bnl/base/export.hpp>

#include <array>
#include <cstdint>
#include <utility>

namespace bnl {
namespace http3 {

namespace setting {

static constexpr uint64_t max_header_list_size = 0x6U;
static constexpr uint64_t num_placeholders = 0x9U;
static constexpr uint64_t qpack_max_table_capacity = 0x1U;
static constexpr uint64_t qpack_blocked_streams = 0x7U;

}

struct BNL_BASE_EXPORT settings {
  static constexpr size_t size = 4;

  uint64_t max_header_list_size = (0x40ULL << 56U) - 1;
  uint64_t num_placeholders = 0;
  uint64_t qpack_max_table_capacity = 0;
  uint64_t qpack_blocked_streams = 0;

  std::array<std::pair<uint64_t, uint64_t>, size> array() const noexcept;
};

}
}
