#include <bnl/quic/event.hpp>

namespace bnl {
namespace quic {

event::event() noexcept // NOLINT
  : type_(type::error)
  , id(0)
  , fin(true)
  , error()
{}

event::event(uint64_t id, bool fin, payload::data data) noexcept // NOLINT
  : type_(type::data)
  , id(id)
  , fin(fin)
  , data(std::move(data))
{}

event::event(uint64_t id, bool fin, payload::error error) noexcept // NOLINT
  : type_(type::error)
  , id(id)
  , fin(fin)
  , error(error)
{}

event::event(const event& other) noexcept // NOLINT
  : type_(other.type_)
  , id(other.id)
  , fin(other.fin)
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

event::event(event&& other) noexcept // NOLINT
  : type_(other.type_)
  , id(other.id)
  , fin(other.fin)
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
      data.~buffer();
      break;
    case type::error:
      error.~error_code();
      break;
  }
}

event::operator event::type() const noexcept
{
  return type_;
}

} // namespace quic
} // namespace bnl
