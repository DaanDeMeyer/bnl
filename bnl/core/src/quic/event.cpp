#include <bnl/quic/event.hpp>

namespace bnl {
namespace quic {

event::event() noexcept // NOLINT
    : type_(type::error), error()
{}

event::event(payload::data data) noexcept // NOLINT
    : type_(type::data), data(std::move(data))
{}

event::event(payload::error error) noexcept // NOLINT
    : type_(type::error), error(error)
{}

event::event(const event &other) noexcept // NOLINT
    : type_(other.type_)
{
  switch (type_) {
    case type::data:
      new (&data) payload::data(other.data);
      break;
    case type::error:
      new (&error) payload::error(other.error);
      break;
  }
}

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

} // namespace quic
} // namespace bnl