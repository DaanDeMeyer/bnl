#pragma once

#include <bnl/http3/endpoint/stream/request.hpp>
#include <bnl/http3/export.hpp>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace client {
namespace stream {
namespace request {

class BNL_HTTP3_EXPORT sender : public endpoint::stream::request::sender {
public:
  using endpoint::stream::request::sender::sender;
};

class BNL_HTTP3_EXPORT receiver : public endpoint::stream::request::receiver {
public:
  receiver(uint64_t id, const log::api *logger) noexcept;

private:
  base::result<event> process(frame frame) noexcept final;

private:
  const log::api *logger_;
};

}
}
}
}
}
