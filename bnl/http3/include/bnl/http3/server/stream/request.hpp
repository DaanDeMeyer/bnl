#pragma once

#include <bnl/http3/endpoint/stream/request.hpp>
#include <bnl/http3/export.hpp>

namespace bnl {
namespace http3 {
namespace server {
namespace stream {
namespace request {

class BNL_HTTP3_EXPORT sender : public endpoint::stream::request::sender {
public:
  using endpoint::stream::request::sender::sender;
};

class BNL_HTTP3_EXPORT receiver : public endpoint::stream::request::receiver {
public:
  explicit receiver(uint64_t id) noexcept;

  using endpoint::stream::request::receiver::process;

private:
  result<event> process(frame frame) noexcept final;
};

}
}
}
}
}
