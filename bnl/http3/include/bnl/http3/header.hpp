#pragma once

#include <bnl/buffer.hpp>
#include <bnl/http3/export.hpp>

namespace bnl {
namespace http3 {

class BNL_HTTP3_EXPORT header_view {
public:
  header_view(buffer_view name, buffer_view value);

  buffer_view name() const noexcept;
  buffer_view value() const noexcept;

private:
  buffer_view name_;
  buffer_view value_;
};

struct BNL_HTTP3_EXPORT header {
  buffer name;
  buffer value;

  operator header_view() const noexcept; // NOLINT
};

BNL_HTTP3_EXPORT bool
operator==(header_view first, header_view second) noexcept;

BNL_HTTP3_EXPORT bool
operator!=(header_view first, header_view second) noexcept;

} // namespace http3
} // namespace bnl
