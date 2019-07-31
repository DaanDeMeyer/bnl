#pragma once

#include <bnl/http3/event.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/result.hpp>

namespace bnl {
namespace http3 {
namespace endpoint {

template<typename Connection>
class BNL_HTTP3_EXPORT generator {
public:
  generator(uint64_t id, Connection &connection) noexcept
    : id_(id)
    , connection_(connection)
  {}

  bool next() noexcept
  {
    // If the last event was `finished`, we're done.
    if (finished_) {
      return false;
    }

    result<event> result = connection_.process(id_);

    destroy(r);
    new (&r) http3::result<event>(std::move(result));

    // If the stream needs more data, we're done.
    if (!r && r.error() == http3::error::incomplete) {
      return false;
    }

    if (r && r.value() == event::type::finished) {
      finished_ = true;
    }

    return true;
  }

  result<event> get()
  {
    result<event> result = std::move(r);

    destroy(r);
    new (&r) http3::result<event>(error::internal);

    return result;
  }

private:
  template<typename T>
  void destroy(T &t)
  {
    t.~T();
  }

  http3::result<event> r = base::failure(error::internal);
  bool finished_ = false;

  uint64_t id_;
  Connection &connection_;
};

}
}
}
