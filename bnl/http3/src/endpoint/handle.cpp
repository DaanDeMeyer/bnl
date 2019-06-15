#include <bnl/http3/endpoint/handle.hpp>

#include <bnl/http3/endpoint/shared/request.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace endpoint {

handle::handle(uint64_t id,
               shared::request::sender *ref,
               const log::api *logger) noexcept
    : id_(id), ref_(ref), logger_(logger)
{}

handle::handle(handle &&other) noexcept : handle()
{
  operator=(std::move(other));
}

handle &handle::operator=(handle &&other) noexcept
{
  std::swap(ref_, other.ref_);

  if (ref_ != nullptr) {
    ref_->handle_ = this;
  }

  return *this;
}

handle::~handle() noexcept
{
  if (ref_ == nullptr) {
    return;
  }

  ref_->handle_ = nullptr;
}

bool handle::valid() const noexcept
{
  return ref_ != nullptr;
}

uint64_t handle::id() const noexcept
{
  return id_;
}

void handle::header(header_view header, std::error_code &ec)
{
  if (ref_ == nullptr) {
    THROW_VOID(error::stream_closed);
  }

  return ref_->headers_.add(header, ec);
}

void handle::start(std::error_code &ec) noexcept
{
  if (ref_ == nullptr) {
    THROW_VOID(error::stream_closed);
  }

  return ref_->headers_.fin(ec);
}

void handle::body(buffer body, std::error_code &ec)
{
  if (ref_ == nullptr) {
    THROW_VOID(error::stream_closed);
  }

  return ref_->body_.add(std::move(body), ec);
}

void handle::fin(std::error_code &ec) noexcept
{
  if (ref_ == nullptr) {
    THROW_VOID(error::stream_closed);
  }

  return ref_->body_.fin(ec);
}

} // namespace endpoint
} // namespace http3
} // namespace bnl