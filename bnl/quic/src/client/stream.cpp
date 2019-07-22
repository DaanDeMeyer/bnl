#include <bnl/quic/client/stream.hpp>

#include <bnl/base/error.hpp>
#include <bnl/quic/client/ngtcp2/connection.hpp>
#include <bnl/quic/client/ngtcp2/error.hpp>
#include <bnl/util/enum.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace quic {
namespace client {

stream::stream(uint64_t id, ngtcp2::connection *ngtcp2, const log::api *logger)
  : id_(id)
  , ngtcp2_(ngtcp2)
  , logger_(logger)
{}

result<base::buffer>
stream::send()
{
  if (buffers_.empty()) {
    return base::error::idle;
  }

  if (!opened()) {
    result<void> r = ngtcp2_->open(id_);
    if (!r) {
      return r.error() == ngtcp2::error::stream_id_blocked
               ? base::error::idle
               : std::move(r).error();
    }

    opened_ = true;
  }

  base::buffer &first = buffers_.front();
  bool fin = fin_ && (first == buffers_.back());

  base::buffer packet;
  size_t stream_bytes_written = 0;

  result<std::pair<base::buffer, uint64_t>> r =
    ngtcp2_->write_stream(id_, first, fin);

  if (!r) {
    if (r.error() == ngtcp2::error::stream_id_blocked ||
        r.error() == ngtcp2::error::stream_data_blocked) {
      return base::error::idle;
    }

    return std::move(r).error();
  }

  std::tie(packet, stream_bytes_written) = std::move(r).value();

  base::buffer sent = buffers_.slice(stream_bytes_written);
  keepalive_.push(std::move(sent));

  return packet;
}

result<void>
stream::add(base::buffer buffer)
{
  assert(!fin_);

  buffers_.push(std::move(buffer));

  return bnl::success();
}

result<void>
stream::fin()
{
  assert(!fin_);

  fin_ = true;

  return bnl::success();
}

result<void>
stream::ack(size_t size)
{
  if (size > keepalive_.size()) {
    LOG_E("ngtcp2's acked stream ({}) data ({}) exceeds remaining data ({})",
          id_,
          size,
          keepalive_.size());
    THROW(quic::connection::error::internal);
  }

  keepalive_.consume(size);

  return bnl::success();
}

bool
stream::finished() const noexcept
{
  return fin_ && buffers_.empty() && keepalive_.empty();
}

bool
stream::opened() const noexcept
{
  return opened_;
}

}
}
}
