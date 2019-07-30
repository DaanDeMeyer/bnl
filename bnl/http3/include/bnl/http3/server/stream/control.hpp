#pragma once

#include <bnl/http3/endpoint/stream/control.hpp>
#include <bnl/http3/export.hpp>

namespace bnl {
namespace http3 {
namespace server {
namespace stream {
namespace control {

class BNL_HTTP3_EXPORT sender : public endpoint::stream::control::sender {
public:
  explicit sender() noexcept;
};

class BNL_HTTP3_EXPORT receiver : public endpoint::stream::control::receiver {
public:
  explicit receiver() noexcept;

  using endpoint::stream::control::receiver::process;

private:
  result<event> process(frame frame) noexcept final;
};

}
}
}
}
}
