#include <bnl/http3/codec/frame.hpp>

namespace bnl {
namespace http3 {

frame::frame() noexcept // NOLINT
  : type_(frame::type::data)
{}

frame::frame(frame::payload::data data) noexcept // NOLINT
  : type_(frame::type::data)
  , data(data)
{}

frame::frame(frame::payload::headers headers) noexcept // NOLINT
  : type_(frame::type::headers)
  , headers(headers)
{}

frame::frame(frame::payload::priority priority) noexcept // NOLINT
  : type_(frame::type::priority)
  , priority(priority)
{}

frame::frame(frame::payload::settings settings) noexcept // NOLINT
  : type_(frame::type::settings)
  , settings(settings)
{}

frame::frame(frame::payload::cancel_push cancel_push) noexcept // NOLINT
  : type_(frame::type::cancel_push)
  , cancel_push(cancel_push)
{}

frame::frame(frame::payload::push_promise push_promise) noexcept // NOLINT
  : type_(frame::type::push_promise)
  , push_promise(push_promise)
{}

frame::frame(frame::payload::goaway goaway) noexcept // NOLINT
  : type_(frame::type::goaway)
  , goaway(goaway)
{}

frame::frame(frame::payload::max_push_id max_push_id) noexcept // NOLINT
  : type_(frame::type::max_push_id)
  , max_push_id(max_push_id)
{}

frame::frame(frame::payload::duplicate_push duplicate_push) noexcept // NOLINT
  : type_(frame::type::duplicate_push)
  , duplicate_push(duplicate_push)
{}

frame::operator frame::type() const noexcept
{
  return type_;
}

bool
operator==(const frame &lhs, const frame &rhs)
{
  if (lhs.type_ != rhs.type_) {
    return false;
  }

  switch (lhs) {
    case frame::type::data:
      return lhs.data.size == rhs.data.size;

    case frame::type::headers:
      return lhs.headers.size == rhs.headers.size;

    case frame::type::priority:
      return lhs.priority.element_dependency_id ==
               rhs.priority.element_dependency_id &&
             lhs.priority.element_dependency_type ==
               rhs.priority.element_dependency_type &&
             lhs.priority.prioritized_element_id ==
               rhs.priority.prioritized_element_id &&
             lhs.priority.prioritized_element_type ==
               rhs.priority.prioritized_element_type &&
             lhs.priority.weight == rhs.priority.weight;

    case frame::type::cancel_push:
      return lhs.cancel_push.push_id == rhs.cancel_push.push_id;

    case frame::type::settings:
      return lhs.settings.array() == rhs.settings.array();

    case frame::type::push_promise:
      return lhs.push_promise.push_id == rhs.push_promise.push_id &&
             lhs.push_promise.size == rhs.push_promise.size;

    case frame::type::goaway:
      return lhs.goaway.stream_id == rhs.goaway.stream_id;

    case frame::type::max_push_id:
      return lhs.max_push_id.push_id == rhs.max_push_id.push_id;

    case frame::type::duplicate_push:
      return lhs.duplicate_push.push_id == rhs.duplicate_push.push_id;
  }

  return false;
}

bool
operator!=(const frame &lhs, const frame &rhs)
{
  return !(lhs == rhs);
}

}
}
