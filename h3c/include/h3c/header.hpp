#pragma once

#include <h3c/buffer.hpp>

namespace h3c {

class header_view {
public:
  header_view(buffer_view name, buffer_view value);

  H3C_EXPORT buffer_view name() const noexcept;
  H3C_EXPORT buffer_view value() const noexcept;

private:
  buffer_view name_;
  buffer_view value_;
};

struct header {
  buffer name;
  buffer value;

  H3C_EXPORT operator header_view() const noexcept; // NOLINT
};

H3C_EXPORT bool operator==(header_view first, header_view second) noexcept;

H3C_EXPORT bool operator!=(header_view first, header_view second) noexcept;

} // namespace h3c
