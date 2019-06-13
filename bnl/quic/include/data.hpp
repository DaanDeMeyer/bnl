#pragma once

#include <bnl/quic/export.hpp>

#include <bnl/buffer.hpp>

#include <functional>
#include <system_error>

namespace bnl {
namespace quic {

class data {
  using handler = const std::function<void(data, std::error_code &ec)> &;
};

} // namespace quic
} // namespace bnl