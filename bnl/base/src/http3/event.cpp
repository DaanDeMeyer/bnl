#include <bnl/http3/event.hpp>

namespace bnl {
namespace http3 {

event::event(payload::settings settings) noexcept
  : type_(event::type::settings)
  , settings(settings)
{}

event::event(payload::header header) noexcept // NOLINT
  : type_(event::type::header)
  , header(std::move(header))
{}

event::event(payload::body body) noexcept // NOLINT
  : type_(event::type::body)
  , body(std::move(body))
{}

event::event(event &&other) noexcept // NOLINT
  : type_(other.type_)
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
  }
}

event::~event() noexcept
{
  switch (type_) {
    case event::type::settings:
      settings.~settings();
      break;
    case event::type::header:
      using header = payload::header;
      this->header.~header();
      break;
    case event::type::body:
      body.~data();
      break;
  }
}

event::operator event::type() const noexcept
{
  return type_;
}

}
}
