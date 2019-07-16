#include <bnl/http3/event.hpp>

namespace bnl {
namespace http3 {

event::event() noexcept // NOLINT
  : type_(event::type::error)
  , id(0)
  , fin(true)
  , error()
{}

event::event(uint64_t id, // NOLINT
             bool fin,
             payload::settings settings) noexcept
  : type_(event::type::settings)
  , id(id)
  , fin(fin)
  , settings(settings)
{}

event::event(uint64_t id, bool fin, payload::header header) noexcept // NOLINT
  : type_(event::type::header)
  , id(id)
  , fin(fin)
  , header(std::move(header))
{}

event::event(uint64_t id, bool fin, payload::body body) noexcept // NOLINT
  : type_(event::type::body)
  , id(id)
  , fin(fin)
  , body(std::move(body))
{}

event::event(uint64_t id, bool fin, payload::error error) noexcept // NOLINT
  : type_(event::type::error)
  , id(id)
  , fin(fin)
  , error(error)
{}

event::event(const event &other) noexcept // NOLINT
  : type_(other.type_)
  , id(other.id)
  , fin(other.fin)
{
  switch (type_) {
    case event::type::settings:
      new (&settings) payload::settings(other.settings);
      break;
    case event::type::header:
      new (&header) payload::header(other.header);
      break;
    case event::type::body:
      new (&body) payload::body(other.body);
      break;
    case event::type::error:
      new (&error) payload::error(other.error);
      break;
  }
}

event::event(event &&other) noexcept // NOLINT
  : type_(other.type_)
  , id(other.id)
  , fin(other.fin)
{
  switch (type_) {
    case event::type::settings:
      new (&settings) payload::settings(other.settings);
      break;
    case event::type::header:
      new (&header) payload::header(std::move(other.header));
      break;
    case event::type::body:
      new (&body) payload::body(std::move(other.body));
      break;
    case event::type::error:
      new (&error) payload::error(other.error);
      break;
  }
}

event::~event() noexcept
{
  switch (type_) {
    case event::type::settings:
      settings.~settings();
      break;
    case event::type::header:
      header.~header();
      break;
    case event::type::body:
      body.~buffer();
      break;
    case event::type::error:
      error.~error_code();
      break;
  }
}

event::operator event::type() const noexcept
{
  return type_;
}

} // namespace http3
} // namespace bnl
