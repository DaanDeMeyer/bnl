#include <bnl/quic/event.hpp>

#include <fmt/ostream.h>

namespace bnl {
namespace quic {

event::event(payload::data data) noexcept // NOLINT
  : type_(type::data)
  , data(std::move(data))
{}

event::event(payload::error error) noexcept // NOLINT
  : type_(type::error)
  , error(error)
{}

event::event(event &&other) noexcept // NOLINT
  : type_(other.type_)
{
  switch (type_) {
    case type::data:
      new (&data) payload::data(std::move(other.data));
      break;
    case type::error:
      new (&error) payload::error(other.error);
      break;
  }
}

event::~event() noexcept
{
  switch (type_) {
    case type::data:
      data.~data();
      break;
    case type::error:
      error.~error();
      break;
  }
}

event::operator event::type() const noexcept
{
  return type_;
}

std::ostream &
operator<<(std::ostream &os, const event &event)
{
  switch (event) {
    case event::type::data:
      fmt::print(os,
                 "DATA ((id: {}, fin: {}, size: {})",
                 event.data.id,
                 event.data.fin,
                 event.data.buffer.size());
      break;
    case event::type::error:
      fmt::print(os, "ERROR ({})", event.error.value);
      break;
  }

  return os;
}

}
}
