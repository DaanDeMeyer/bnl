#include <h3c/frame.hpp>

#include <cassert>

namespace h3c {

frame::frame() noexcept : type_(frame::type::data){} // NOLINT

frame::frame(frame::payload::data data) noexcept // NOLINT
    : type_(frame::type::data), data_(data)
{}

frame::frame(frame::payload::headers headers) noexcept // NOLINT
    : type_(frame::type::headers), headers_(headers)
{}

frame::frame(frame::payload::priority priority) noexcept // NOLINT
    : type_(frame::type::priority), priority_(priority)
{}

frame::frame(frame::payload::settings settings) noexcept // NOLINT
    : type_(frame::type::settings), settings_(settings)
{}

frame::frame(frame::payload::cancel_push cancel_push) noexcept // NOLINT
    : type_(frame::type::cancel_push), cancel_push_(cancel_push)
{}

frame::frame(frame::payload::push_promise push_promise) noexcept // NOLINT
    : type_(frame::type::push_promise), push_promise_(push_promise)
{}

frame::frame(frame::payload::goaway goaway) noexcept // NOLINT
    : type_(frame::type::goaway), goaway_(goaway)
{}

frame::frame(frame::payload::max_push_id max_push_id) noexcept // NOLINT
    : type_(frame::type::max_push_id), max_push_id_(max_push_id)
{}

frame::frame(frame::payload::duplicate_push duplicate_push) noexcept // NOLINT
    : type_(frame::type::duplicate_push), duplicate_push_(duplicate_push)
{}

frame::operator frame::type() const noexcept
{
  return type_;
}

frame::payload::data frame::data() const noexcept
{
  assert(type_ == frame::type::data);
  return data_;
}

frame::payload::headers frame::headers() const noexcept
{
  assert(type_ == frame::type::headers);
  return headers_;
}

frame::payload::priority frame::priority() const noexcept
{
  assert(type_ == frame::type::priority);
  return priority_;
}

frame::payload::settings frame::settings() const noexcept
{
  assert(type_ == frame::type::settings);
  return settings_;
}

frame::payload::cancel_push frame::cancel_push() const noexcept
{
  assert(type_ == frame::type::cancel_push);
  return cancel_push_;
}

frame::payload::push_promise frame::push_promise() const noexcept
{
  assert(type_ == frame::type::push_promise);
  return push_promise_;
}

frame::payload::goaway frame::goaway() const noexcept
{
  assert(type_ == frame::type::goaway);
  return goaway_;
}

frame::payload::max_push_id frame::max_push_id() const noexcept
{
  assert(type_ == frame::type::max_push_id);
  return max_push_id_;
}

frame::payload::duplicate_push frame::duplicate_push() const noexcept
{
  assert(type_ == frame::type::duplicate_push);
  return duplicate_push_;
}

} // namespace h3c
