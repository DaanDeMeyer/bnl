#include <bnl/http3/event.hpp>

namespace bnl {
namespace http3 {

event::event(payload::settings settings) noexcept // NOLINE
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

event::event(payload::finished finished) noexcept // NOLINE
  : type_(event::type::finished)
  , finished(finished)
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
    case event::type::finished:
      new (&finished) payload::finished(other.finished);
      break;
  }
}

// `header.~header();` doesn't work in clang
// (https://bugs.llvm.org/show_bug.cgi?id=12350) so we use a template function
// to circumvent it.
template<class T>
void
destroy(T &t)
{
  t.~T();
}

event::~event() noexcept
{
  switch (type_) {
    case event::type::settings:
      destroy(settings);
      break;
    case event::type::header:
      destroy(header);
      break;
    case event::type::body:
      destroy(body);
      break;
    case event::type::finished:
      destroy(finished);
      break;
  }
}

event::operator event::type() const noexcept
{
  return type_;
}

}
}
