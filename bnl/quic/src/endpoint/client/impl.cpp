#include <endpoint/client/impl.hpp>

#include <bnl/base/error.hpp>
#include <bnl/quic/error.hpp>
#include <bnl/util/enum.hpp>
#include <bnl/util/error.hpp>

#include <algorithm>

namespace bnl {
namespace quic {
namespace endpoint {
namespace client {

impl::impl(path path,
           const params &params,
           clock clock,
           const log::api *logger) noexcept
  : prng_(std::random_device()())
  , ngtcp2_(path, params, this, std::move(clock), prng_, logger)
  , handshake_(ngtcp2_.dcid(), &ngtcp2_, logger)
  , path_(path)
  , logger_(logger)
{}

std::error_code
impl::client_initial()
{
  LOG_T("handshake: client initial");
  return handshake_.send();
}

std::error_code
impl::recv_crypto_data(crypto::level level, base::buffer_view data)
{
  LOG_T("handshake: recv crypto data: {}", data.size());

  std::error_code ec = handshake_.recv(level, data);

  return ec == base::error::incomplete ? base::error::success : ec;
}

void
impl::handshake_completed()
{
  LOG_I("handshake: finished");
}

void
impl::recv_stream_data(uint64_t id, bool fin, base::buffer_view data)
{
  base::buffer buffer(data.size());
  std::copy_n(data.data(), data.size(), buffer.data());

  event_buffer_.emplace_back(id, fin, std::move(buffer));
}

std::error_code
impl::acked_crypto_offset(crypto::level level, size_t size)
{
  LOG_T("handshake: acked crypto offset: {}", size);
  return handshake_.ack(level, size);
}

std::error_code
impl::acked_stream_data_offset(uint64_t id, size_t size)
{
  auto match = ngtcp2_streams_keepalive_.find(id);

  if (match == ngtcp2_streams_keepalive_.end()) {
    LOG_E("ngtcp2 acked data for stream {} which does not exist in the send "
          "buffer",
          id);
    THROW(base::error::internal);
  }

  base::buffers &buffers = match->second;

  if (size > buffers.size()) {
    LOG_E("ngtcp2's acked stream ({}) data ({}) exceeds remaining data ({})",
          id,
          size,
          buffers.size());
    THROW(base::error::internal);
  }

  buffers.consume(size);

  return {};
}

void
impl::stream_opened(uint64_t id)
{
  LOG_I("stream opened: {}", id);
}

void
impl::stream_closed(uint64_t id, uint64_t error)
{
  std::error_code ec = make_error_code(static_cast<quic::error>(error));
  LOG_I("stream closed: {} (reason: {})", id, ec.message());
}

void
impl::stream_reset(uint64_t id, size_t final_size, uint64_t error)
{
  (void)final_size;

  std::error_code ec = make_error_code(static_cast<quic::error>(error));
  LOG_I("Stream reset: {} (reason: {})", id, ec.message());
}

std::error_code
impl::recv_stateless_reset(base::buffer_view bytes, base::buffer_view token)
{
  (void)bytes;
  (void)token;

  LOG_T("received stateless reset");

  THROW(base::error::not_implemented);
}

std::error_code
impl::recv_retry(base::buffer_view dcid)
{
  (void)dcid;

  LOG_T("received retry");

  THROW(base::error::not_implemented);
}

void
impl::extend_max_local_streams_bidi(uint64_t max_streams)
{
  LOG_T("max local bidi streams: {}", max_streams);

  max_local_bidi_streams_ = max_streams;
}

void
impl::extend_max_local_streams_uni(uint64_t max_streams)
{
  LOG_T("max local uni streams: {}", max_streams);

  max_local_uni_streams_ = max_streams;
}

void
impl::extend_max_remote_streams_bidi(uint64_t max_streams)
{
  LOG_I("max remote bidi streams: {}", max_streams);

  max_remote_bidi_streams_ = max_streams;
}

void
impl::extend_max_remote_streams_uni(uint64_t max_streams)
{
  LOG_I("max remote uni streams: {}", max_streams);

  max_remote_uni_streams_ = max_streams;
}

void
impl::random(base::buffer_view_mut dest)
{
  auto dis = std::uniform_int_distribution<uint8_t>(0, 255);
  auto f = [&dis, this]() { return dis(prng_); };

  std::generate(dest.begin(), dest.end(), f);
}

void
impl::new_connection_id(base::buffer_view_mut dest)
{
  random(dest);
}

void
impl::new_stateless_reset_token(base::buffer_view_mut dest)
{
  random(dest);
}

void
impl::remove_connection_id(base::buffer_view cid)
{
  (void)cid;
}

std::error_code
impl::update_key()
{
  return handshake_.update_keys();
}

std::error_code
impl::path_validation(base::buffer_view local,
                      base::buffer_view remote,
                      bool succeeded)
{
  (void)local;
  (void)remote;

  CHECK(succeeded, error::path_validation);

  return {};
}

std::error_code
impl::select_preferred_address(base::buffer_view_mut dest,
                               ip::endpoint ipv4,
                               ip::endpoint ipv6,
                               base::buffer_view token)
{
  (void)token;

  switch (path_.local().address()) {
    case ip::address::type::ipv4:
      std::copy_n(ipv4.address().bytes().data(),
                  ipv4.address().bytes().size(),
                  dest.data());
      break;

    case ip::address::type::ipv6:
      std::copy_n(ipv6.address().bytes().data(),
                  ipv6.address().bytes().size(),
                  dest.data());
      break;

    default:
      assert(false);
  }

  return {};
}

void
impl::extend_max_stream_data(uint64_t id, uint64_t max_data)
{
  LOG_I("stream ({}) max data: {}", id, max_data);
}

base::result<base::buffer>
impl::send()
{
  TRY(handshake_.send());

  {
    base::result<base::buffer> result = ngtcp2_.write_pkt();
    if (result) {
      return std::move(result).value();
    }

    CHECK(result == base::error::idle, result.error());
  }

  {
  }

  THROW(base::error::idle);
}

std::error_code
impl::recv(base::buffer_view data, event::handler handler)
{
  TRY(ngtcp2_.read_pkt(data));

  std::error_code ec;

  while (!ec && !event_buffer_.empty()) {
    ec = handler(std::move(event_buffer_.front()));
    event_buffer_.pop_front();
  }

  return ec;
}

void
impl::add(quic::event event)
{
  (void)event;
  // TODO
}

duration
impl::timeout() const noexcept
{
  return ngtcp2_.timeout();
}

duration
impl::expiry() const noexcept
{
  return ngtcp2_.expiry();
}

std::error_code
impl::expire()
{
  return ngtcp2_.expire();
}

base::result<crypto>
impl::crypto() const noexcept
{
  return handshake_.negotiated_crypto();
}

const log::api *
impl::logger() const noexcept
{
  return logger_;
}

} // namespace client
} // namespace endpoint
} // namespace quic
} // namespace bnl
