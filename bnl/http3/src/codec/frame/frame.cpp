#include <bnl/http3/codec/frame.hpp>

namespace bnl {
namespace http3 {

frame::frame() noexcept : type_(frame::type::data) {} // NOLINT

frame::frame(frame::payload::data data) noexcept // NOLINT
    : type_(frame::type::data), data(data)
{}

frame::frame(frame::payload::headers headers) noexcept // NOLINT
    : type_(frame::type::headers), headers(headers)
{}

frame::frame(frame::payload::priority priority) noexcept // NOLINT
    : type_(frame::type::priority), priority(priority)
{}

frame::frame(frame::payload::settings settings) noexcept // NOLINT
    : type_(frame::type::settings), settings(settings)
{}

frame::frame(frame::payload::cancel_push cancel_push) noexcept // NOLINT
    : type_(frame::type::cancel_push), cancel_push(cancel_push)
{}

frame::frame(frame::payload::push_promise push_promise) noexcept // NOLINT
    : type_(frame::type::push_promise), push_promise(push_promise)
{}

frame::frame(frame::payload::goaway goaway) noexcept // NOLINT
    : type_(frame::type::goaway), goaway(goaway)
{}

frame::frame(frame::payload::max_push_id max_push_id) noexcept // NOLINT
    : type_(frame::type::max_push_id), max_push_id(max_push_id)
{}

frame::frame(frame::payload::duplicate_push duplicate_push) noexcept // NOLINT
    : type_(frame::type::duplicate_push), duplicate_push(duplicate_push)
{}

frame::operator frame::type() const noexcept
{
  return type_;
}

} // namespace http3
} // namespace bnl
