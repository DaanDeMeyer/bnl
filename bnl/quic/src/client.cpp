#include <bnl/quic/client.hpp>

#include <endpoint/client/impl.hpp>

namespace bnl {
namespace quic {

client::client(path path,
               const params &params,
               clock clock,
               const log::api *logger) noexcept
  : impl_(std::unique_ptr<endpoint::client::impl>(
      new endpoint::client::impl(path, params, std::move(clock), logger)))
{}

client::client(client &&) noexcept = default;

client &
client::operator=(client &&) noexcept = default;

client::~client() noexcept = default;

base::result<base::buffer>
client::send()
{
  return impl_->send();
}

std::error_code
client::recv(base::buffer_view data, event::handler handler)
{
  return impl_->recv(data, handler);
}

void
client::add(quic::event event)
{
  return impl_->add(std::move(event));
}

duration
client::timeout() const noexcept
{
  return impl_->timeout();
}

duration
client::expiry() const noexcept
{
  return impl_->expiry();
}

std::error_code
client::expire()
{
  return impl_->expire();
}

} // namespace quic
} // namespace bnl
