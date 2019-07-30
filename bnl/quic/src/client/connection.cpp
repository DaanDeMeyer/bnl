#include <bnl/quic/client/connection.hpp>

#include <bnl/base/error.hpp>
#include <bnl/log.hpp>

namespace bnl {
namespace quic {
namespace client {

generator::generator(connection &connection)
  : connection_(connection)
{}

bool
generator::next()
{
  return !connection_.event_buffer_.empty();
}

bnl::result<quic::event>
generator::result()
{
  bnl::result<event> result = std::move(connection_.event_buffer_.front());
  connection_.event_buffer_.pop_front();
  return result;
}

connection::connection(const ip::host &host,
                       path path,
                       const params &params,
                       clock clock) noexcept
  : prng_(std::random_device()())
  , ngtcp2_(path, params, this, std::move(clock), prng_)
  , handshake_(host, ngtcp2_.dcid(), &ngtcp2_)
  , path_(path)
{}

result<void>
connection::client_initial()
{
  BNL_LOG_T("handshake: client initial");
  return handshake_.send();
}

result<void>
connection::recv_crypto_data(crypto::level level, base::buffer_view data)
{
  BNL_LOG_T("handshake: recv crypto data: {}", data.size());

  result<void> r = handshake_.recv(level, data);

  if (r || r.error() == base::error::incomplete) {
    return success();
  }

  return std::move(r).error();
}

void
connection::handshake_completed()
{
  BNL_LOG_I("handshake: finished");
}

void
connection::recv_stream_data(uint64_t id, bool fin, base::buffer_view data)
{
  base::buffer buffer(data.size());
  std::copy_n(data.data(), data.size(), buffer.data());

  event_buffer_.emplace_back(quic::data{ id, fin, std::move(buffer) });
}

result<void>
connection::acked_crypto_offset(crypto::level level, size_t size)
{
  BNL_LOG_T("handshake: acked crypto offset: {}", size);
  return handshake_.ack(level, size);
}

result<void>
connection::acked_stream_data_offset(uint64_t id, size_t size)
{
  auto match = streams_.find(id);
  if (match == streams_.end()) {
    BNL_LOG_E("ngtcp2 acked data for stream {} which does not exist", id);
    return quic::connection::error::internal;
  }

  stream &stream = match->second;
  BNL_TRY(stream.ack(size));

  if (stream.finished()) {
    streams_.erase(id);
  }

  return success();
}

void
connection::stream_opened(uint64_t id)
{
  BNL_LOG_I("stream opened: {}", id);
}

void
connection::stream_closed(uint64_t id, uint64_t error)
{
  BNL_LOG_I("stream closed: {} (reason: {})", id, error);
}

void
connection::stream_reset(uint64_t id, uint64_t final_size, uint64_t error)
{
  (void) final_size;

  event_buffer_.emplace_back(
    event::payload::error{ application::error::rst_stream, id, error });
  BNL_LOG_I("Stream reset: {} (reason: {})", id, error);
}

result<void>
connection::recv_stateless_reset(base::buffer_view bytes,
                                 base::buffer_view token)
{
  (void) bytes;
  (void) token;

  BNL_LOG_T("received stateless reset");

  return error::not_implemented;
}

result<void>
connection::recv_retry(base::buffer_view dcid)
{
  (void) dcid;

  BNL_LOG_T("received retry");

  return error::not_implemented;
}

void
connection::extend_max_local_streams_bidi(uint64_t max_streams)
{
  BNL_LOG_T("max local bidi streams: {}", max_streams);

  max_local_bidi_streams_ = max_streams;
}

void
connection::extend_max_local_streams_uni(uint64_t max_streams)
{
  BNL_LOG_T("max local uni streams: {}", max_streams);

  max_local_uni_streams_ = max_streams;
}

void
connection::extend_max_remote_streams_bidi(uint64_t max_streams)
{
  BNL_LOG_I("max remote bidi streams: {}", max_streams);

  max_remote_bidi_streams_ = max_streams;
}

void
connection::extend_max_remote_streams_uni(uint64_t max_streams)
{
  BNL_LOG_I("max remote uni streams: {}", max_streams);

  max_remote_uni_streams_ = max_streams;
}

void
connection::random(base::buffer_view_mut dest)
{
  auto dis = std::uniform_int_distribution<uint8_t>(0, 255);
  auto f = [&dis, this]() { return dis(prng_); };

  std::generate(dest.begin(), dest.end(), f);
}

void
connection::new_connection_id(base::buffer_view_mut dest)
{
  random(dest);
}

void
connection::new_stateless_reset_token(base::buffer_view_mut dest)
{
  random(dest);
}

void
connection::remove_connection_id(base::buffer_view cid)
{
  (void) cid;
}

result<void>
connection::update_key()
{
  return handshake_.update_keys();
}

result<void>
connection::path_validation(base::buffer_view local,
                            base::buffer_view peer,
                            bool succeeded)
{
  (void) local;
  (void) peer;

  if (!succeeded) {
    return error::path_validation;
  }

  return success();
}

result<void>
connection::select_preferred_address(base::buffer_view_mut dest,
                                     ip::endpoint ipv4,
                                     ip::endpoint ipv6,
                                     base::buffer_view token)
{
  (void) token;

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

  return success();
}

void
connection::extend_max_stream_data(uint64_t id, uint64_t max_data)
{
  BNL_LOG_I("stream ({}) max data: {}", id, max_data);
}

result<base::buffer>
connection::send()
{
  BNL_TRY(handshake_.send());

  {
    result<base::buffer> r = ngtcp2_.write_pkt();
    if (r) {
      return r;
    }

    if (r.error() != base::error::idle) {
      return std::move(r).error();
    }
  }

  if (!handshake_.completed()) {
    return base::error::idle;
  }

  {
    for (auto &entry : streams_) {
      stream &stream = entry.second;

      result<base::buffer> r = stream.send();
      if (r) {
        return r;
      }

      if (r.error() != base::error::idle) {
        return std::move(r).error();
      }
    }
  }

  return base::error::idle;
}

result<generator>
connection::recv(base::buffer_view data)
{
  result<void> r = ngtcp2_.read_pkt(data);

  if (!r) {
    event_buffer_.emplace_back(std::move(r).error());
  }

  return generator(*this);
}

result<void>
connection::add(event event)
{
  switch (event) {
    case event::type::data:
      return add(std::move(event.data));
    case event::type::error:
      return error::not_implemented;
  }

  assert(false);
  return success();
}

result<void>
connection::add(quic::data data) // NOLINT
{
  auto match = streams_.find(data.id);

  if (match == streams_.end()) {
    stream stream(data.id, &ngtcp2_);
    streams_.insert(std::make_pair(data.id, std::move(stream)));
  }

  stream &stream = streams_.at(data.id);
  BNL_TRY(stream.add(std::move(data.buffer)));

  if (data.fin) {
    BNL_TRY(stream.fin());
  }

  return success();
}

duration
connection::timeout() const noexcept
{
  return ngtcp2_.timeout();
}

duration
connection::expiry() const noexcept
{
  return ngtcp2_.expiry();
}

result<void>
connection::expire()
{
  return ngtcp2_.expire();
}

result<crypto>
connection::crypto() const noexcept
{
  return handshake_.negotiated_crypto();
}

}
}
}
