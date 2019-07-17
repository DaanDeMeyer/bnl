#include <bnl/quic/client/connection.hpp>

#include <bnl/util/error.hpp>

namespace bnl {
namespace quic {
namespace client {

connection::connection(path path,
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
connection::client_initial()
{
  LOG_T("handshake: client initial");
  return handshake_.send();
}

std::error_code
connection::recv_crypto_data(crypto::level level, base::buffer_view data)
{
  LOG_T("handshake: recv crypto data: {}", data.size());

  std::error_code ec = handshake_.recv(level, data);

  return ec == base::error::incomplete ? base::error::success : ec;
}

void
connection::handshake_completed()
{
  LOG_I("handshake: finished");
}

void
connection::recv_stream_data(uint64_t id, bool fin, base::buffer_view data)
{
  base::buffer buffer(data.size());
  std::copy_n(data.data(), data.size(), buffer.data());

  event_buffer_.emplace_back(id, fin, std::move(buffer));
}

std::error_code
connection::acked_crypto_offset(crypto::level level, size_t size)
{
  LOG_T("handshake: acked crypto offset: {}", size);
  return handshake_.ack(level, size);
}

std::error_code
connection::acked_stream_data_offset(uint64_t id, size_t size)
{
  auto match = streams_.find(id);
  if (match == streams_.end()) {
    LOG_E("ngtcp2 acked data for stream {} which does not exist", id);
    THROW(base::error::internal);
  }

  stream &stream = match->second;
  stream.ack(size);

  if (stream.finished()) {
    streams_.erase(id);
  }

  return {};
}

void
connection::stream_opened(uint64_t id)
{
  LOG_I("stream opened: {}", id);
}

void
connection::stream_closed(uint64_t id, uint64_t error)
{
  std::error_code ec = make_error_code(static_cast<quic::error>(error));
  LOG_I("stream closed: {} (reason: {})", id, ec.message());
}

void
connection::stream_reset(uint64_t id, uint64_t final_size, uint64_t error)
{
  (void)final_size;

  std::error_code ec = make_error_code(static_cast<quic::error>(error));
  LOG_I("Stream reset: {} (reason: {})", id, ec.message());
}

std::error_code
connection::recv_stateless_reset(base::buffer_view bytes,
                                 base::buffer_view token)
{
  (void)bytes;
  (void)token;

  LOG_T("received stateless reset");

  THROW(base::error::not_implemented);
}

std::error_code
connection::recv_retry(base::buffer_view dcid)
{
  (void)dcid;

  LOG_T("received retry");

  THROW(base::error::not_implemented);
}

void
connection::extend_max_local_streams_bidi(uint64_t max_streams)
{
  LOG_T("max local bidi streams: {}", max_streams);

  max_local_bidi_streams_ = max_streams;
}

void
connection::extend_max_local_streams_uni(uint64_t max_streams)
{
  LOG_T("max local uni streams: {}", max_streams);

  max_local_uni_streams_ = max_streams;
}

void
connection::extend_max_remote_streams_bidi(uint64_t max_streams)
{
  LOG_I("max remote bidi streams: {}", max_streams);

  max_remote_bidi_streams_ = max_streams;
}

void
connection::extend_max_remote_streams_uni(uint64_t max_streams)
{
  LOG_I("max remote uni streams: {}", max_streams);

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
  (void)cid;
}

std::error_code
connection::update_key()
{
  return handshake_.update_keys();
}

std::error_code
connection::path_validation(base::buffer_view local,
                            base::buffer_view peer,
                            bool succeeded)
{
  (void)local;
  (void)peer;

  CHECK(succeeded, error::path_validation);

  return {};
}

std::error_code
connection::select_preferred_address(base::buffer_view_mut dest,
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
connection::extend_max_stream_data(uint64_t id, uint64_t max_data)
{
  LOG_I("stream ({}) max data: {}", id, max_data);
}

base::result<base::buffer>
connection::send()
{
  TRY(handshake_.send());

  {
    base::result<base::buffer> result = ngtcp2_.write_pkt();
    if (result) {
      return result;
    }

    CHECK(result == base::error::idle, result.error());
  }

  {
    for (auto &entry : streams_) {
      stream &stream = entry.second;

      base::result<base::buffer> result = stream.send();
      if (result != base::error::idle) {
        return result;
      }
    }
  }

  THROW(base::error::idle);
}

std::error_code
connection::recv(base::buffer_view data, event::handler handler)
{
  TRY(ngtcp2_.read_pkt(data));

  std::error_code ec;

  while (!ec && !event_buffer_.empty()) {
    ec = handler(std::move(event_buffer_.front()));
    event_buffer_.pop_front();
  }

  return ec;
}

std::error_code
connection::add(quic::event event) // NOLINT
{
  uint64_t id = event.id;
  if (streams_.find(id) == streams_.end()) {
    stream stream(id, &ngtcp2_, logger_);
    streams_.insert(std::make_pair(id, std::move(stream)));
  }

  stream &stream = streams_.at(id);

  TRY(stream.add(std::move(event.data)));

  if (event.fin) {
    TRY(stream.fin());
  }

  return {};
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

std::error_code
connection::expire()
{
  return ngtcp2_.expire();
}

base::result<crypto>
connection::crypto() const noexcept
{
  return handshake_.negotiated_crypto();
}

const log::api *
connection::logger() const noexcept
{
  return logger_;
}

base::result<uint64_t>
connection::open_bidi_stream()
{
  base::result<uint64_t> result = ngtcp2_.open_bidi_stream();

  if (result) {
    stream stream(result.value(), &ngtcp2_, logger_);
    streams_.insert(std::make_pair(result.value(), std::move(stream)));
  }

  return result;
}
}
}
}
