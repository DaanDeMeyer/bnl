#pragma once

#include <bnl/quic/export.hpp>

#include <bnl/buffer.hpp>

#include <system_error>

namespace bnl {
namespace quic {
namespace transport {

struct BNL_QUIC_EXPORT data {
  
  buffer buffer;
};

}
}
}